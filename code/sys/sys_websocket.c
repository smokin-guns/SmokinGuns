#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include <emscripten.h>

EM_JS(int, Sys_WebSocketSendPacket, (const void *data, int length), {
	if (!globalThis.SGWebTransport || length < 0 || length > 1400)
		return 0;
	return globalThis.SGWebTransport.send(HEAPU8.slice(data, data + length)) ? 1 : 0;
});

EM_JS(int, Sys_WebSocketReceivePacket, (byte *destination, int capacity), {
	if (!globalThis.SGWebTransport || capacity <= 0)
		return 0;
	const packet = globalThis.SGWebTransport.receive();
	if (!packet)
		return 0;
	if (packet.byteLength > capacity)
		return -1;
	HEAPU8.set(packet, destination);
	return packet.byteLength;
});
