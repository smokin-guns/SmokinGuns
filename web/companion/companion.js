"use strict";

// Local browser companion: serves the web build, discovers public servers over
// UDP, and relays selected game packets between WebSocket and server UDP.
const crypto = require("node:crypto");
const dgram = require("node:dgram");
const fs = require("node:fs");
const http = require("node:http");
const path = require("node:path");
const { URL } = require("node:url");

const host = "127.0.0.1";
const port = Number(process.env.SG_COMPANION_PORT || 8765);
const masterHost = process.env.SG_MASTER || "master.smokin-guns.org";
const masterPort = Number(process.env.SG_MASTER_PORT || 27950);
// Smokin' Guns currently advertises legacy protocol 68 by default. Set
// SG_PROTOCOL=71 when using servers configured for the newer protocol.
const protocol = process.env.SG_PROTOCOL || "68";
const webRoot = path.resolve(process.env.SG_WEB_ROOT || path.join(__dirname, "../../build/release-emscripten-wasm32"));
const originAllowlist = new Set((process.env.SG_ALLOWED_ORIGINS || "").split(",").map((item) => item.trim()).filter(Boolean));
const configuredServers = (process.env.SG_SERVERS || "").split(",").map((item) => item.trim()).filter(Boolean);
const maxDatagramBytes = 1400;
const maxPacketsPerSecond = 120;
const servers = new Map();
const sessions = new Map();
let pairingCode = crypto.randomBytes(16).toString("hex");
let pairUsed = false;
let refreshStarted = 0;

if (!Number.isInteger(port) || port < 1 || port > 65535) throw new Error("SG_COMPANION_PORT must be a valid TCP port");
const mimeTypes = {
  ".html": "text/html; charset=utf-8", ".js": "text/javascript; charset=utf-8",
  ".wasm": "application/wasm", ".data": "application/octet-stream", ".pk3": "application/octet-stream",
  ".css": "text/css; charset=utf-8", ".txt": "text/plain; charset=utf-8", ".json": "application/json; charset=utf-8",
  ".png": "image/png", ".ico": "image/x-icon", ".svg": "image/svg+xml"
};

function sendJson(response, status, value) {
  const body = Buffer.from(JSON.stringify(value));
  response.writeHead(status, { "content-type": "application/json; charset=utf-8", "content-length": body.length,
    "cache-control": "no-store", "x-content-type-options": "nosniff" });
  response.end(body);
}

function authorized(request) {
  const token = request.headers.authorization || "";
  return token.startsWith("Bearer ") && (sessions.get(token.slice(7)) || 0) > Date.now();
}

function allowedOrigin(origin) {
  if (!origin || origin === "null") return false;
  return originAllowlist.size === 0 || originAllowlist.has(origin);
}

function parseInfo(text) {
  const result = {};
  const parts = text.replace(/^\\/, "").split("\\");
  for (let i = 0; i + 1 < parts.length; i += 2) result[parts[i]] = parts[i + 1];
  return result;
}

function parseMasterResponse(packet) {
  const header = Buffer.from("getserversResponse\0");
  const offset = packet.indexOf(header);
  if (offset < 0) return [];
  let cursor = offset + header.length;
  const found = [];
  while (cursor < packet.length) {
    if (packet[cursor] === 0x5c) {
      if (packet.subarray(cursor, cursor + 4).toString() === "\\EOT") break;
      if (cursor + 7 > packet.length) break;
      const ip = [...packet.subarray(cursor + 1, cursor + 5)].join(".");
      const serverPort = packet.readUInt16BE(cursor + 5);
      if (serverPort) found.push({ host: ip, port: serverPort });
      cursor += 7;
    } else {
      cursor++;
    }
  }
  return found;
}

function parseServerEndpoint(value) {
  const match = /^([a-zA-Z0-9.-]+):(\d{1,5})$/.exec(value);
  if (!match) throw new Error(`Invalid SG_SERVERS endpoint: ${value}`);
  const serverPort = Number(match[2]);
  if (serverPort < 1 || serverPort > 65535) throw new Error(`Invalid SG_SERVERS port: ${value}`);
  return { host: match[1], port: serverPort };
}

const fixedEndpoints = configuredServers.map(parseServerEndpoint);

function queryServer(endpoint, timeoutMs = 900) {
  return new Promise((resolve) => {
    const socket = dgram.createSocket("udp4");
    const started = Date.now();
    let done = false;
    const finish = (value) => {
      if (done) return;
      done = true;
      clearTimeout(timer);
      try { socket.close(); } catch (_error) {}
      resolve(value);
    };
    const timer = setTimeout(() => {
      console.warn(`No getinfo response from ${endpoint.host}:${endpoint.port} over UDP.`);
      finish(null);
    }, timeoutMs);
    socket.on("error", (error) => {
      console.warn(`UDP query to ${endpoint.host}:${endpoint.port} failed: ${error.message}`);
      finish(null);
    });
    socket.on("message", (packet) => {
      const responseHeader = Buffer.from("infoResponse\n");
      const offset = packet.indexOf(responseHeader);
      if (offset < 0) {
        console.warn(`Unexpected UDP reply from ${endpoint.host}:${endpoint.port}: ${packet.subarray(4).toString("latin1").replace(/\0/g, " ").slice(0, 100)}`);
        finish(null);
        return;
      }
      const info = parseInfo(packet.subarray(offset + responseHeader.length).toString("latin1").replace(/\0+$/, "").trim());
      if (info.gamename !== "smokinguns" || info.protocol !== protocol) {
        console.warn(`Server ${endpoint.host}:${endpoint.port} reports game=${info.gamename || "?"}, protocol=${info.protocol || "?"}; companion expects smokinguns/${protocol}.`);
        finish(null);
        return;
      }
      finish({ ...endpoint, id: `${endpoint.host.replaceAll(".", "-")}-${endpoint.port}`, name: info.hostname || `${endpoint.host}:${endpoint.port}`,
        map: info.mapname || "", players: Number(info.clients || 0), maxPlayers: Number(info.sv_maxclients || 0),
        ping: Date.now() - started, game: info.gamename });
    });
    socket.send(Buffer.concat([Buffer.alloc(4, 0xff), Buffer.from("getinfo sgweb\0")]), endpoint.port, endpoint.host);
  });
}

async function refreshServers() {
  const now = Date.now();
  if (now - refreshStarted < 5000) return;
  refreshStarted = now;
  if (fixedEndpoints.length) {
    const results = await Promise.all(fixedEndpoints.map((endpoint) => queryServer(endpoint)));
    servers.clear();
    for (const item of results) if (item) servers.set(item.id, item);
    console.log(`Discovery updated: ${servers.size} responding configured server(s).`);
    return;
  }
  const master = dgram.createSocket("udp4");
  const packetPromise = new Promise((resolve) => {
    const packets = [];
    let quietTimer;
    const deadline = setTimeout(() => finish(), 4000);
    function finish() {
      clearTimeout(deadline);
      clearTimeout(quietTimer);
      try { master.close(); } catch (_error) {}
      resolve(packets);
    }
    master.on("message", (packet) => {
      packets.push(packet);
      clearTimeout(quietTimer);
      if (packet.includes(Buffer.from("\\EOT"))) finish();
      else quietTimer = setTimeout(finish, 250);
    });
    master.on("error", finish);
    master.send(Buffer.concat([Buffer.alloc(4, 0xff), Buffer.from(`getservers smokinguns ${protocol} full empty\0`)]), masterPort, masterHost);
  });
  const responses = await packetPromise;
  if (!responses.length) {
    servers.clear();
    console.log("Discovery returned no response from the configured master server.");
    return;
  }
  const unique = new Map();
  for (const response of responses) for (const endpoint of parseMasterResponse(response)) {
    unique.set(`${endpoint.host}:${endpoint.port}`, endpoint);
  }
  const endpoints = [...unique.values()].slice(0, 256);
  const results = await Promise.all(endpoints.map((endpoint) => queryServer(endpoint)));
  servers.clear();
  for (const item of results) if (item) servers.set(item.id, item);
  console.log(`Discovery updated: ${servers.size} responding Smokin' Guns servers from ${endpoints.length} addresses.`);
}

function encodeFrame(opcode, payload) {
  let header;
  if (payload.length < 126) header = Buffer.from([0x80 | opcode, payload.length]);
  else { header = Buffer.alloc(4); header[0] = 0x80 | opcode; header[1] = 126; header.writeUInt16BE(payload.length, 2); }
  return Buffer.concat([header, payload]);
}

function parseFrames(buffer, callback) {
  let offset = 0;
  while (buffer.length - offset >= 2) {
    const first = buffer[offset], second = buffer[offset + 1];
    const opcode = first & 15;
    let length = second & 127, head = 2;
    if (!(first & 128) || !(second & 128) || (first & 112)) throw new Error("invalid frame");
    if (length === 126) { if (buffer.length - offset < 4) break; length = buffer.readUInt16BE(offset + 2); head = 4; }
    if (length > maxDatagramBytes || (opcode >= 8 && length > 125)) throw new Error("frame too large");
    if (buffer.length - offset < head + 4 + length) break;
    const mask = buffer.subarray(offset + head, offset + head + 4);
    const body = Buffer.from(buffer.subarray(offset + head + 4, offset + head + 4 + length));
    for (let i = 0; i < body.length; i++) body[i] ^= mask[i & 3];
    callback(opcode, body);
    offset += head + 4 + length;
  }
  return buffer.subarray(offset);
}

const httpServer = http.createServer(async (request, response) => {
  const url = new URL(request.url, `http://${host}:${port}`);
  if (url.pathname.startsWith("/api/")) {
    const origin = request.headers.origin;
    if (!allowedOrigin(origin)) { sendJson(response, 403, { error: "This browser origin is not allowed by the companion" }); return; }
    response.setHeader("access-control-allow-origin", origin);
    response.setHeader("vary", "Origin");
    if (request.method === "OPTIONS") {
      const headers = { "access-control-allow-methods": "GET, POST, OPTIONS", "access-control-allow-headers": "authorization, content-type" };
      if (request.headers["access-control-request-private-network"] === "true") headers["access-control-allow-private-network"] = "true";
      response.writeHead(204, headers);
      response.end(); return;
    }
    if (url.pathname === "/api/pair" && request.method === "POST") {
      let body = "";
      request.on("data", (chunk) => { body += chunk; if (body.length > 1024) request.destroy(); });
      request.on("end", () => {
        let supplied;
        try { supplied = JSON.parse(body).code; } catch (_error) { sendJson(response, 400, { error: "Invalid pairing request" }); return; }
        if (pairUsed || typeof supplied !== "string" || supplied !== pairingCode) { sendJson(response, 403, { error: "Pairing code is invalid or already used" }); return; }
        pairUsed = true;
        pairingCode = "";
        const token = crypto.randomBytes(24).toString("hex");
        sessions.set(token, Date.now() + 12 * 60 * 60 * 1000);
        sendJson(response, 200, { token, expiresIn: 43200 });
      });
      return;
    }
    if (!authorized(request)) { sendJson(response, 401, { error: "Pair this browser with the companion first" }); return; }
    if (url.pathname === "/api/servers" && request.method === "GET") {
      sendJson(response, 200, { servers: [...servers.values()].map(({ id, name, map, players, maxPlayers, ping }) => ({ id, name, map, players, maxPlayers, ping })) }); return;
    }
    if (url.pathname === "/api/refresh" && request.method === "POST") {
      await refreshServers();
      sendJson(response, 200, { servers: [...servers.values()].length }); return;
    }
    sendJson(response, 404, { error: "Not found" }); return;
  }

  const relative = decodeURIComponent(url.pathname === "/" ? "/index.html" : url.pathname);
  const liveWebFiles = { "/index.html": path.join(__dirname, "../index.html"), "/transport.js": path.join(__dirname, "../transport.js") };
  const candidate = liveWebFiles[relative] || path.resolve(webRoot, `.${relative}`);
  const file = candidate.startsWith(`${webRoot}${path.sep}`) || liveWebFiles[relative] ? candidate : "";
  if (relative === "/" && !fs.existsSync(path.join(webRoot, "index.html"))) {
    response.writeHead(200, { "content-type": "text/plain; charset=utf-8", "cache-control": "no-store" });
    response.end("Smokin' Guns local companion is running. Open the Smokin' Guns web app, paste the pairing key printed in this terminal, then search for servers.\n");
    return;
  }
  if (!file || !fs.existsSync(file) || !fs.statSync(file).isFile()) {
    response.writeHead(404); response.end("Not found"); return;
  }
  response.writeHead(200, { "content-type": mimeTypes[path.extname(file)] || "application/octet-stream",
    "x-content-type-options": "nosniff", "cache-control": path.extname(file) === ".wasm" || path.extname(file) === ".data" ? "public, max-age=3600" : "no-cache" });
  fs.createReadStream(file).pipe(response);
});

httpServer.on("upgrade", (request, socket) => {
  const url = new URL(request.url, `http://${host}:${port}`);
  const match = /^\/udp\/([0-9-]+)$/.exec(url.pathname);
  const endpoint = match && servers.get(match[1]);
  const protocols = (request.headers["sec-websocket-protocol"] || "").split(",").map((item) => item.trim());
  const token = protocols[1];
  if (!endpoint || protocols.length !== 2 || protocols[0] !== "smokinguns" || !sessions.has(token) ||
      sessions.get(token) < Date.now() || !allowedOrigin(request.headers.origin) ||
      request.headers.upgrade?.toLowerCase() !== "websocket" || request.headers["sec-websocket-version"] !== "13") {
    socket.end("HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n"); return;
  }
  const accept = crypto.createHash("sha1").update(request.headers["sec-websocket-key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").digest("base64");
  socket.write(["HTTP/1.1 101 Switching Protocols", "Upgrade: websocket", "Connection: Upgrade", `Sec-WebSocket-Accept: ${accept}`, "Sec-WebSocket-Protocol: smokinguns", "\r\n"].join("\r\n"));
  const udp = dgram.createSocket("udp4");
  let buffered = Buffer.alloc(0), window = Date.now(), packetCount = 0, disposed = false;
  const dispose = () => { if (disposed) return; disposed = true; try { udp.close(); } catch (_error) {} };
  udp.on("message", (message) => { if (!socket.destroyed && message.length <= maxDatagramBytes) socket.write(encodeFrame(2, message)); });
  udp.on("error", () => { if (!socket.destroyed) socket.destroy(); dispose(); });
  socket.on("data", (chunk) => {
    buffered = Buffer.concat([buffered, chunk]);
    try { buffered = parseFrames(buffered, (opcode, payload) => {
      if (opcode === 8) { socket.end(encodeFrame(8, payload)); dispose(); return; }
      if (opcode === 9) { socket.write(encodeFrame(10, payload)); return; }
      if (opcode !== 2 || payload.length === 0) { socket.destroy(); dispose(); return; }
      const now = Date.now(); if (now - window >= 1000) { window = now; packetCount = 0; }
      if (++packetCount > maxPacketsPerSecond) { socket.destroy(); dispose(); return; }
      udp.send(payload, endpoint.port, endpoint.host);
    }); } catch (_error) { socket.destroy(); dispose(); }
  });
  socket.on("close", dispose); socket.on("error", dispose);
});

httpServer.listen(port, host, () => {
  console.log(`Smokin' Guns companion listening at http://${host}:${port}`);
  console.log(`Pairing key (one use): ${pairingCode}`);
  if (fs.existsSync(path.join(webRoot, "index.html"))) console.log(`Serving local web build: ${webRoot}`);
  else console.log("Proxy-only mode: open the hosted Smokin' Guns web app in your browser.");
});

setInterval(() => { for (const [token, expires] of sessions) if (expires < Date.now()) sessions.delete(token); }, 60 * 60 * 1000).unref();
