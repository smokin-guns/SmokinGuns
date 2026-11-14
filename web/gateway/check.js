"use strict";

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const gateway = fs.readFileSync(path.join(__dirname, "server.js"), "utf8");
const transport = fs.readFileSync(path.join(__dirname, "..", "transport.js"), "utf8");
const test = fs.readFileSync(path.join(__dirname, "test.js"), "utf8");

assert.match(gateway, /Set SG_SERVER to one fixed host:port endpoint/);
assert.match(gateway, /Set SG_GATEWAY_TOKEN/);
assert.match(gateway, /Set SG_ALLOWED_ORIGINS/);
assert.match(gateway, /requestPath\.pathname !== "\/udp" \|\| requestPath\.search/);
assert.match(gateway, /maxDatagramBytes = 1400/);
assert.match(gateway, /maxPacketsPerSecond = 120/);
assert.match(gateway, /maxSessionsPerAddress = 8/);
assert.match(gateway, /udpSocket\.send\(payload, endpoint\.port, endpoint\.host\)/);
assert.match(transport, /new WebSocket\(url, \["smokinguns", accessToken\]\)/);
assert.match(transport, /onDatagram\(new Uint8Array\(event\.data\)\)/);
assert.match(test, /UDP forwarding/);

console.log("gateway static security and wiring checks: ok");
