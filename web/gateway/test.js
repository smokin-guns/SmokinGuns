"use strict";

const assert = require("node:assert/strict");
const crypto = require("node:crypto");
const dgram = require("node:dgram");
const net = require("node:net");
const { spawn } = require("node:child_process");
const { once } = require("node:events");

const token = "test-token-for-local-checks";
const testOrigin = "http://localhost:8080";
const gatewaySocketPath = `/tmp/smokinguns-gateway-${process.pid}.sock`;

async function freePort() {
  const server = net.createServer();
  server.listen(0, "127.0.0.1");
  await once(server, "listening");
  const { port } = server.address();
  await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
  return port;
}

function maskedBinaryFrame(payload) {
  const mask = crypto.randomBytes(4);
  let header;
  if (payload.length < 126) {
    header = Buffer.from([0x82, 0x80 | payload.length]);
  } else {
    header = Buffer.alloc(4);
    header[0] = 0x82;
    header[1] = 0x80 | 126;
    header.writeUInt16BE(payload.length, 2);
  }
  const encoded = Buffer.from(payload);
  for (let index = 0; index < encoded.length; index++) encoded[index] ^= mask[index & 3];
  return Buffer.concat([header, mask, encoded]);
}

function parseServerFrame(buffer) {
  if (buffer.length < 2) return null;
  const opcode = buffer[0] & 0x0f;
  let length = buffer[1] & 0x7f;
  let headerLength = 2;
  if (length === 126) {
    if (buffer.length < 4) return null;
    length = buffer.readUInt16BE(2);
    headerLength = 4;
  } else if (length === 127) {
    if (buffer.length < 10) return null;
    const wideLength = buffer.readBigUInt64BE(2);
    if (wideLength > BigInt(Number.MAX_SAFE_INTEGER)) throw new Error("frame too large");
    length = Number(wideLength);
    headerLength = 10;
  }
  if (buffer.length < headerLength + length) return null;
  return { opcode, payload: buffer.subarray(headerLength, headerLength + length), consumed: headerLength + length };
}

async function main() {
  const gamePort = await freePort();
  const udpServer = dgram.createSocket("udp4");
  udpServer.bind(gamePort, "127.0.0.1");
  await once(udpServer, "listening");

  const gateway = spawn(process.execPath, ["server.js"], {
    cwd: __dirname,
    env: {
      ...process.env,
      SG_GATEWAY_BIND_ADDRESS: gatewaySocketPath,
      SG_SERVER: `127.0.0.1:${gamePort}`,
      SG_GATEWAY_TOKEN: token,
      SG_ALLOWED_ORIGINS: testOrigin
    },
    stdio: ["ignore", "pipe", "pipe"]
  });
  let gatewayOutput = "";
  gateway.stdout.on("data", (chunk) => { gatewayOutput += chunk; });
  gateway.stderr.on("data", (chunk) => { gatewayOutput += chunk; });

  try {
    const deadline = Date.now() + 5000;
    while (!gatewayOutput.includes("Forwarding only to") && Date.now() < deadline) {
      await new Promise((resolve) => setTimeout(resolve, 20));
      if (gateway.exitCode !== null) throw new Error(`gateway exited early: ${gatewayOutput}`);
    }
    assert.match(gatewayOutput, /Forwarding only to/);

    const unauthorizedSocket = net.connect(gatewaySocketPath);
    let unauthorizedResponse = Buffer.alloc(0);
    unauthorizedSocket.write([
      "GET /udp HTTP/1.1",
      "Host: localhost",
      "Upgrade: websocket",
      "Connection: Upgrade",
      `Origin: ${testOrigin}`,
      "Sec-WebSocket-Version: 13",
      `Sec-WebSocket-Key: ${crypto.randomBytes(16).toString("base64")}`,
      "\r\n"
    ].join("\r\n"));
    unauthorizedSocket.on("data", (chunk) => { unauthorizedResponse = Buffer.concat([unauthorizedResponse, chunk]); });
    await once(unauthorizedSocket, "close");
    assert.match(unauthorizedResponse.toString(), /^HTTP\/1\.1 401 Unauthorized/);

    const udpPacketPromise = once(udpServer, "message");
    const socket = net.connect(gatewaySocketPath);
    await once(socket, "connect");
    const websocketKey = crypto.randomBytes(16).toString("base64");
    const handshake = [
      "GET /udp HTTP/1.1",
      "Host: localhost",
      "Upgrade: websocket",
      "Connection: Upgrade",
      `Origin: ${testOrigin}`,
      "Sec-WebSocket-Version: 13",
      `Sec-WebSocket-Key: ${websocketKey}`,
      `Sec-WebSocket-Protocol: smokinguns, ${token}`,
      "\r\n"
    ].join("\r\n");

    let received = Buffer.alloc(0);
    let upgraded = false;
    let resolveFrame;
    const framePromise = new Promise((resolve) => { resolveFrame = resolve; });
    socket.on("data", (chunk) => {
      received = Buffer.concat([received, chunk]);
      if (!upgraded) {
        const headerEnd = received.indexOf("\r\n\r\n");
        if (headerEnd < 0) return;
        const header = received.subarray(0, headerEnd).toString();
        assert.match(header, /^HTTP\/1\.1 101 Switching Protocols/m);
        const expectedAccept = crypto.createHash("sha1").update(websocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").digest("base64");
        assert.ok(header.includes(`Sec-WebSocket-Accept: ${expectedAccept}`));
        received = received.subarray(headerEnd + 4);
        upgraded = true;
      }
      const frame = parseServerFrame(received);
      if (frame) resolveFrame(frame);
    });
    socket.write(handshake);

    const packet = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x67, 0x65, 0x74, 0x73, 0x74, 0x61, 0x74, 0x75, 0x73]);
    socket.write(maskedBinaryFrame(packet));
    const [receivedPacket, sender] = await udpPacketPromise;
    assert.deepEqual(receivedPacket, packet);

    udpServer.send(Buffer.from([1, 2, 3, 4]), sender.port, sender.address);
    const frame = await Promise.race([
      framePromise,
      new Promise((_, reject) => setTimeout(() => reject(new Error("gateway did not return UDP packet")), 2000))
    ]);
    assert.equal(frame.opcode, 0x2);
    assert.deepEqual(frame.payload, Buffer.from([1, 2, 3, 4]));
    socket.destroy();
    console.log("gateway auth, UDP forwarding, and WebSocket return path: ok");
  } finally {
    gateway.kill("SIGTERM");
    udpServer.close();
  }
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
