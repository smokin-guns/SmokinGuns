"use strict";

const crypto = require("node:crypto");
const dgram = require("node:dgram");
const http = require("node:http");

const listenHost = process.env.SG_GATEWAY_HOST || "127.0.0.1";
const listenPort = Number(process.env.SG_GATEWAY_PORT || 8081);
const originAllowlist = new Set((process.env.SG_ALLOWED_ORIGINS || "")
  .split(",").map((value) => value.trim()).filter(Boolean));
const configuredServer = process.env.SG_SERVER || "";
const accessToken = process.env.SG_GATEWAY_TOKEN || "";
const maxDatagramBytes = 1400;
const maxPacketsPerSecond = 120;
const maxBufferedBytes = 64 * 1024;
const maxSessionsPerAddress = 8;
const websocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const configuredBindAddress = process.env.SG_GATEWAY_BIND_ADDRESS || "";

if (!configuredServer) throw new Error("Set SG_SERVER to one fixed host:port endpoint");
if (!accessToken) throw new Error("Set SG_GATEWAY_TOKEN to a high-entropy access token");
if (originAllowlist.size === 0) throw new Error("Set SG_ALLOWED_ORIGINS to one or more allowed browser origins");
if (!Number.isInteger(listenPort) || listenPort < 1 || listenPort > 65535) {
  throw new Error("SG_GATEWAY_PORT must be between 1 and 65535");
}

function parseEndpoint(value) {
  const match = /^([a-zA-Z0-9.-]+):(\d{1,5})$/.exec(value);
  if (!match) throw new Error(`Invalid server endpoint: ${value}`);
  const port = Number(match[2]);
  if (port < 1 || port > 65535) throw new Error(`Invalid server port: ${value}`);
  return { host: match[1], port, key: `${match[1]}:${port}` };
}

const endpoint = parseEndpoint(configuredServer);
const udpType = endpoint.host.includes(":") ? "udp6" : "udp4";
const sessionsByAddress = new Map();

function rejectUpgrade(socket, status, reason) {
  socket.end(`HTTP/1.1 ${status} ${reason}\r\nConnection: close\r\n\r\n`);
}

function parseFrames(buffer, onFrame) {
  let offset = 0;
  while (buffer.length - offset >= 2) {
    const first = buffer[offset];
    const second = buffer[offset + 1];
    const fin = (first & 0x80) !== 0;
    const opcode = first & 0x0f;
    const masked = (second & 0x80) !== 0;
    let length = second & 0x7f;
    let headerLength = 2;

    if (!fin || !masked || (first & 0x70) !== 0) throw new Error("invalid websocket frame");
    if (length === 126) {
      if (buffer.length - offset < 4) break;
      length = buffer.readUInt16BE(offset + 2);
      headerLength = 4;
    } else if (length === 127) {
      if (buffer.length - offset < 10) break;
      const wideLength = buffer.readBigUInt64BE(offset + 2);
      if (wideLength > BigInt(maxDatagramBytes)) throw new Error("frame too large");
      length = Number(wideLength);
      headerLength = 10;
    }

    if (length > maxDatagramBytes) throw new Error("frame too large");
    const isControl = opcode >= 0x8;
    if (isControl && (length > 125 || !fin)) throw new Error("invalid control frame");
    const frameLength = headerLength + 4 + length;
    if (buffer.length - offset < frameLength) break;

    const mask = buffer.subarray(offset + headerLength, offset + headerLength + 4);
    const payload = Buffer.from(buffer.subarray(offset + headerLength + 4, offset + frameLength));
    for (let index = 0; index < payload.length; index++) payload[index] ^= mask[index & 3];
    onFrame(opcode, payload);
    offset += frameLength;
  }
  return buffer.subarray(offset);
}

function encodeFrame(opcode, payload) {
  const length = payload.length;
  let header;
  if (length < 126) {
    header = Buffer.from([0x80 | opcode, length]);
  } else if (length <= 0xffff) {
    header = Buffer.alloc(4);
    header[0] = 0x80 | opcode;
    header[1] = 126;
    header.writeUInt16BE(length, 2);
  } else {
    header = Buffer.alloc(10);
    header[0] = 0x80 | opcode;
    header[1] = 127;
    header.writeBigUInt64BE(BigInt(length), 2);
  }
  return Buffer.concat([header, payload]);
}

const httpServer = http.createServer((_request, response) => {
  response.writeHead(200, { "content-type": "text/plain; charset=utf-8" });
  response.end("Smokin' Guns WebSocket UDP gateway\n");
});

httpServer.on("upgrade", (request, socket) => {
  let requestPath;
  try {
    requestPath = new URL(request.url, "http://gateway.invalid");
  } catch (_error) {
    rejectUpgrade(socket, 400, "Bad Request");
    return;
  }
  const origin = request.headers.origin;
  const protocols = (request.headers["sec-websocket-protocol"] || "")
    .split(",").map((value) => value.trim());

  if (requestPath.pathname !== "/udp" || requestPath.search) {
    rejectUpgrade(socket, 404, "Not Found");
    return;
  }
  if (!origin || !originAllowlist.has(origin)) {
    rejectUpgrade(socket, 403, "Forbidden");
    return;
  }
  if (protocols.length !== 2 || protocols[0] !== "smokinguns" || protocols[1] !== accessToken) {
    rejectUpgrade(socket, 401, "Unauthorized");
    return;
  }
  if (request.headers.upgrade?.toLowerCase() !== "websocket" ||
      request.headers["sec-websocket-version"] !== "13" ||
      !/^[A-Za-z0-9+/]{22}==$/.test(request.headers["sec-websocket-key"] || "")) {
    rejectUpgrade(socket, 400, "Bad Request");
    return;
  }

  const accept = crypto.createHash("sha1")
    .update(request.headers["sec-websocket-key"] + websocketGuid)
    .digest("base64");
  const remoteAddress = request.socket.remoteAddress || "unknown";
  const activeSessions = sessionsByAddress.get(remoteAddress) || 0;
  if (activeSessions >= maxSessionsPerAddress) {
    rejectUpgrade(socket, 429, "Too Many Requests");
    return;
  }
  sessionsByAddress.set(remoteAddress, activeSessions + 1);
  socket.write([
    "HTTP/1.1 101 Switching Protocols",
    "Upgrade: websocket",
    "Connection: Upgrade",
    `Sec-WebSocket-Accept: ${accept}`,
    "Sec-WebSocket-Protocol: smokinguns",
    "\r\n"
  ].join("\r\n"));

  const udpSocket = dgram.createSocket(udpType);
  let buffered = Buffer.alloc(0);
  let packetWindowStart = Date.now();
  let packetsInWindow = 0;
  let disposed = false;

  const dispose = () => {
    if (disposed) return;
    disposed = true;
    const sessions = (sessionsByAddress.get(remoteAddress) || 1) - 1;
    if (sessions > 0) sessionsByAddress.set(remoteAddress, sessions);
    else sessionsByAddress.delete(remoteAddress);
    udpSocket.close();
  };

  const closeWebSocket = (code, reason) => {
    if (!socket.destroyed) {
      const reasonBytes = Buffer.from(reason).subarray(0, 100);
      const payload = Buffer.alloc(2 + reasonBytes.length);
      payload.writeUInt16BE(code, 0);
      reasonBytes.copy(payload, 2);
      socket.end(encodeFrame(0x8, payload));
    }
    dispose();
  };

  udpSocket.on("message", (message) => {
    if (socket.destroyed || message.length > maxDatagramBytes) return;
    if (socket.writableLength + message.length + 4 > maxBufferedBytes) {
      closeWebSocket(1013, "client is too slow");
      return;
    }
    socket.write(encodeFrame(0x2, message));
  });
  udpSocket.on("error", () => closeWebSocket(1011, "UDP forwarding failed"));

  socket.on("data", (chunk) => {
    buffered = Buffer.concat([buffered, chunk]);
    try {
      buffered = parseFrames(buffered, (opcode, payload) => {
        if (opcode === 0x8) {
          socket.end(encodeFrame(0x8, payload));
          dispose();
          return;
        }
        if (opcode === 0x9) {
          socket.write(encodeFrame(0xA, payload));
          return;
        }
        if (opcode !== 0x2 || payload.length === 0) {
          closeWebSocket(1003, "binary datagrams required");
          return;
        }

        const now = Date.now();
        if (now - packetWindowStart >= 1000) {
          packetWindowStart = now;
          packetsInWindow = 0;
        }
        if (++packetsInWindow > maxPacketsPerSecond) {
          closeWebSocket(1008, "packet rate exceeded");
          return;
        }
        udpSocket.send(payload, endpoint.port, endpoint.host);
      });
    } catch (_error) {
      closeWebSocket(1002, "invalid websocket frame");
    }
  });
  socket.on("close", dispose);
  socket.on("error", dispose);
});

httpServer.listen(configuredBindAddress || listenPort, configuredBindAddress ? undefined : listenHost, () => {
  console.log(`Gateway listening at ws://${listenHost}:${listenPort}/udp`);
  console.log(`Forwarding only to ${endpoint.key}`);
});

function shutdown() {
  httpServer.close(() => process.exit(0));
}

process.once("SIGINT", shutdown);
process.once("SIGTERM", shutdown);
