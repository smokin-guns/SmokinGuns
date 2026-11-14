// WebSocket-to-UDP client bridge for the Emscripten network pump.
var SGWebTransport = (function () {
  "use strict";

  var socket = null;
  var onDatagram = null;
  var maxDatagramBytes = 1400;
  var maxQueuedBytes = 64 * 1024;
  var datagrams = [];
  var queuedBytes = 0;
  var lastError = "";

  function connect(url, accessToken, receiveCallback) {
    if (socket && socket.readyState <= WebSocket.OPEN) {
      throw new Error("Smokin' Guns transport is already connected");
    }
    if (typeof WebSocket === "undefined") {
      throw new Error("WebSocket is unavailable in this browser");
    }
    if (typeof accessToken !== "string" || accessToken.length < 16) {
      throw new TypeError("accessToken must contain at least 16 characters");
    }
    if (receiveCallback !== undefined && typeof receiveCallback !== "function") {
      throw new TypeError("receiveCallback must be a function");
    }

    lastError = "";
    datagrams = [];
    queuedBytes = 0;
    onDatagram = receiveCallback;
    socket = new WebSocket(url, ["smokinguns", accessToken]);
    socket.binaryType = "arraybuffer";
    socket.onopen = function () {
      lastError = "";
    };
    socket.onmessage = function (event) {
      if (!(event.data instanceof ArrayBuffer) || event.data.byteLength > maxDatagramBytes) {
        socket.close(1009, "invalid datagram size");
        return;
      }
      var bytes = new Uint8Array(event.data);
      if (queuedBytes + bytes.byteLength > maxQueuedBytes) {
        lastError = "WebSocket receive queue is full; packet dropped";
        return;
      }
      datagrams.push(bytes);
      queuedBytes += bytes.byteLength;
      if (onDatagram) onDatagram(bytes);
    };
    socket.onclose = function () {
      onDatagram = null;
      datagrams = [];
      queuedBytes = 0;
      socket = null;
    };
    socket.onerror = function () {
      lastError = "WebSocket transport failed";
    };
    return socket;
  }

  function send(bytes) {
    if (!(bytes instanceof Uint8Array) || bytes.byteLength > maxDatagramBytes) {
      throw new RangeError("datagram must be a Uint8Array no larger than 1400 bytes");
    }
    if (!socket || socket.readyState !== WebSocket.OPEN || socket.bufferedAmount > maxQueuedBytes) {
      return false;
    }
    socket.send(bytes);
    return true;
  }

  function close() {
    if (socket) {
      socket.close();
      socket = null;
    }
    onDatagram = null;
    datagrams = [];
    queuedBytes = 0;
  }

  function receive() {
    if (!datagrams.length) return null;
    var packet = datagrams.shift();
    queuedBytes -= packet.byteLength;
    return packet;
  }

  function state() {
    if (!socket) return "closed";
    if (socket.readyState === WebSocket.CONNECTING) return "connecting";
    if (socket.readyState === WebSocket.OPEN) return "open";
    if (socket.readyState === WebSocket.CLOSING) return "closing";
    return "closed";
  }

  function error() {
    return lastError;
  }

  return { connect: connect, send: send, receive: receive, close: close, state: state, error: error };
})();

if (typeof globalThis !== "undefined") globalThis.SGWebTransport = SGWebTransport;
