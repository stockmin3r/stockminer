/*
The MIT License (MIT)

Copyright (c) 2012-2014 Marcin Kelar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
- original: https://github.com/OrionExplorer/c-websocket/blob/master/cWebSockets.c
- with modifications
*/

#include <conf.h>
#include <sha1.h>

#define WEBSOCKET_UUID                  "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define WEBSOCKET_HANDSHAKE_RESPONSE    "HTTP/1.1 101 Switching Protocols\r\n"              \
                                        "Upgrade: websocket\r\n"                            \
                                        "Connection: Upgrade\r\n"                           \
                                        "Set-Cookie: c=%s; Secure; HttpOnly; Path=/ws;\r\n" \
                                        "Sec-WebSocket-Accept: %s\r\n\r\n"

#define OPCODE_CLOSE      8
#define OPCODE_PING       9
#define OPCODE_PONG      10

#define WEBSOCKET_KEYLEN 60
#define MAX_FRAMES       5

bool websocket_handshake(struct connection *connection, char *data)
{
	SHA1Context sha1;
	char        websocket_key[512];
	char        websocket_base64[512];
	char        websocket_response[2048];
	char        hash[SHA1HashSize];
	char       *p, *p2;
	int         nbytes;

	p = strstr(data, "Sec-WebSocket-Key:");
	if (!p)
		return (false);
	p += 19;
	p2 = strchr(p, '\r');
	if (!p2)
		return (false);
	*p2 = 0;
	if (p2-p >= 64)
		return (false);
	strcpy(websocket_key, p);
	strcat(websocket_key, WEBSOCKET_UUID);
	SHA1Reset(&sha1);
	SHA1Input(&sha1,  (unsigned char *)websocket_key, WEBSOCKET_KEYLEN);
 	SHA1Result(&sha1, (unsigned char *)hash);

	base64_encode((unsigned char *)hash, SHA1HashSize, (unsigned char *)websocket_base64);
	nbytes = snprintf(websocket_response, sizeof(websocket_response)-1, WEBSOCKET_HANDSHAKE_RESPONSE, connection->session->user->cookie, websocket_base64);
	openssl_write_sync(connection, websocket_response, nbytes);
	return (true);
}

void websocket_connect_sync(char *host, unsigned int ip_address, char *api, websocket_handler_t handler)
{
	struct connection *connection = zmalloc(sizeof(*connection));
	char request[1024];
	char response[1024];

	if (!connection)
		return;

	if (!openssl_connect_sync(connection, ip_address, 443)) {
		printf(BOLDRED "openssl_connect_sync failure" RESET "\n");
		return;
	}
	snprintf(request, sizeof(request)-1, WEBSOCKET_CLIENT, api, host);
	openssl_write_sync(connection, request, strlen(request));
	openssl_read_sync2(connection, response, sizeof(response)-1);
	printf(BOLDWHITE "%s" RESET "\n", response);
	connection->packet = malloc(16 KB);
	handler(connection);
}

static int
extract_frames(char *packet, struct frame *frames, int packet_length)
{
	struct frame *frame;
	uint64_t      data_length;
	int           nr_frames = 0, mask_offset, frame_length, data_offset, lensize, x;

	frame = frames;
	for (x=0; x<5; x++) {
		data_length = ((unsigned char) packet[1]) & 127;
		printf("%x %x %x data len: %d\n", (unsigned char)packet[0], (unsigned char)packet[1], (unsigned char)packet[2], data_length);
		if (data_length <= 125) {
			mask_offset = 2;
			lensize     = 1;
		} else if (data_length == 126) {
			mask_offset = 4;
			lensize     = 2;
			data_length = htons(*(unsigned short *)(packet+2));
		} else if (data_length == 127) {
			mask_offset = 10;
			lensize     = 8;
			data_length = bswap_64(*(uint64_t *)(packet+2));
		} else {
			return 0;
		}

		data_offset        = mask_offset + 4;
		mask_offset        = mask_offset;
		frame->data        = (unsigned char *)packet+data_offset;
		frame->data_length = data_length;
		frame_length       = data_length + 5 + lensize;
		*(unsigned int *)frame->mask = *(unsigned int *)(packet+mask_offset);
		printf("data_off: %d mask_off: %d frame_len: %d packet_length: %d\n", data_offset, mask_offset, frame_length, packet_length);

		nr_frames++;
		if (frame_length == packet_length)
			break;
		packet_length -= frame_length;
		if (packet_length < 6)
			break;
		frame++;
		packet += frame_length;
	}
	return (nr_frames);
}

int websocket_send_pong(struct connection *connection, char *packet)
{
	int len = packet[1] & 0x7F;
	packet[0] = OPCODE_PING | 1;
	openssl_write_sync(connection, packet, 1);
	return 1;
}

int websocket_recv2(struct connection *connection, char *msg, int mask)
{
	struct frame *frame;
	struct frame  frames[MAX_FRAMES];
	char         *packet      = connection->packet;
	packet_size_t packet_size = 0;
	unsigned char opcode;
	int           nr_frames, x, y;

	packet_size = openssl_read_sync2(connection, packet, 16 KB);
	opcode      = packet[0] & 0x0F;
	if (opcode == OPCODE_CLOSE)
		return -1;
	if (opcode == OPCODE_PING)
		return websocket_send_pong(connection, packet);
printf("packet size: %d\n", packet_size);
	nr_frames = extract_frames(packet, frames, packet_size);
	if (!nr_frames)
		return -1;
	frame = &frames[0];
	if (nr_frames >= MAX_FRAMES)
		return -1;
	for (x=0; x<nr_frames; x++) {
		for (y = 0; y<frame->data_length; y++) {
			if (mask)
				msg[y] = frame->data[y] = (unsigned char)frame->data[y] ^ frame->mask[y%4];
			else
				msg[y] = frame->data[y] = (unsigned char)frame->data[y];
		}
		msg[frame->data_length++] = 0;
		frame->data               = (unsigned char *)msg;
		msg                      += frame->data_length;
		frame++;
	}
	asm("int3");
	return (nr_frames);	
}


int websocket_recv(char *packet, uint64_t packet_length, struct frame *frames, char *msg, int mask)
{
	struct frame *frame;
	unsigned char opcode;
	int           nr_frames, x, y;

	opcode = packet[0];
	if (opcode != 129 ) {
		if (opcode == 136) {
			printf("WEBSOCKET: CLIENT DISCONNECTED\n");
			return -2;
		}
		if (!(opcode & 2)) {
			printf("WEBSOCKET: UNKNOWN ERROR: %x\n", packet[0]);
			return -1;
		}
		printf(BOLDRED "opcode: %d not 129" RESET "\n", opcode);
	}

	nr_frames = extract_frames(packet, frames, packet_length);
	if (!nr_frames)
		return -1;
	frame = &frames[0];
	if (nr_frames >= MAX_FRAMES)
		return -1;
	for (x=0; x<nr_frames; x++) {
		for (y = 0; y<frame->data_length; y++)
			if (mask)
				msg[y] = frame->data[y] = (unsigned char)frame->data[y] ^ frame->mask[y%4];
			else
				msg[y] = frame->data[y] = (unsigned char)frame->data[y];
		msg[frame->data_length++] = 0;
		frame->data               = (unsigned char *)msg;
		msg                      += frame->data_length;
		frame++;
	}
	return (nr_frames);
}


int websocket_send(struct connection *connection, char *data, uint64_t data_length)
{
	unsigned char message[1024 KB];
	unsigned char *msg     = message;
	char           mask[4] = { 0xaa,0xaa,0xaa,0xaa };
	int            data_start_index, boff = 0;

	if ((int)data_length <= 0)
		return 0;

	if (data_length >= sizeof(message))
		msg = (unsigned char *)malloc(data_length+256);

	msg[0] = (unsigned char)129;
	if(data_length <= 125) {
		if (connection->protocol == TLS_CLIENT)
			msg[1] = (unsigned char)data_length|(1<<7);
		else
			msg[1] = (unsigned char)data_length;
		data_start_index = 2;
	} else if( data_length > 125 && data_length <= 65535) {
		if (connection->protocol == TLS_CLIENT)
			msg[1] = 0xfe; // 126|(1<<7) (mask bit is 1<<7)
		else
			msg[1] = 126;
		msg[2] = (unsigned char)((data_length >> 8) & 255);
		msg[3] = (unsigned char)((data_length) & 255);
		data_start_index = 4;
	} else {
		if (connection->protocol == TLS_CLIENT)
			msg[1] = 0xff;
		else
			msg[1] = 127;
		msg[2] = (unsigned char)((data_length >> 56) & 255);
		msg[3] = (unsigned char)((data_length >> 48) & 255);
		msg[4] = (unsigned char)((data_length >> 40) & 255);
		msg[5] = (unsigned char)((data_length >> 32) & 255);
		msg[6] = (unsigned char)((data_length >> 24) & 255);
		msg[7] = (unsigned char)((data_length >> 16) & 255);
		msg[8] = (unsigned char)((data_length >> 8)  & 255);
		msg[9] = (unsigned char)((data_length)       & 255);
		data_start_index = 10;
	}

	if (connection->protocol == TLS_CLIENT) {
		*(unsigned int *)(msg+data_start_index) = *(unsigned int *)mask;
		data_start_index += 4;

		for (int x = 0; x<data_length; x++) {
			msg[boff+data_start_index] = (unsigned char)data[boff] ^ mask[x%4];
			boff++;
		}
	} else {
		memcpy(&msg[data_start_index], data, data_length);
	}
	openssl_write_sync(connection, (char *)msg, data_length + data_start_index);
	if (msg != message)
		free(msg);
	return 0;
}

void websocket_sendx(struct connection *connection, char *packet, int packet_len)
{
	if (packet_len > 65536) {
		websocket_send4(connection, packet, packet_len);
	} else {
		websocket_send2(connection, packet+6, packet_len);
	}
}

// used to work but after the rewrite there have been segfaults, switch off for now until i have more time to look into it
void websockets_sendall(struct session *session, char *packet, int packet_len)
{
	for (int x=0; x<session->nr_websockets; x++) {
		struct connection *connection = session->websockets[x];
		if (!connection || connection->fd == -1)
			continue;
		if (packet_len > 64 KB) {
			websocket_send4(connection, packet,   packet_len);
		} else {
			websocket_send2(connection, packet+6, packet_len);
		}
	}
}

void websockets_sendall_except(struct session *session, struct connection *this_connection, char *packet, int packet_len)
{
	for (int x=0; x<session->nr_websockets; x++) {
		struct connection *connection = session->websockets[x];
		if (!connection || !connection->ssl || connection->fd == -1)
			continue;
		if (connection->fd == this_connection->fd)
			continue;
		websocket_send(connection, packet, packet_len);
	}
}

int websocket_send_huge(struct connection *connection, char *data, uint64_t data_length)
{
	unsigned char message[4280 KB]; // yeah i know
	int data_start_index;

	message[0] = (unsigned char)129;
	message[1] = 127;
	message[2] = (unsigned char)((data_length >> 56) & 255);
	message[3] = (unsigned char)((data_length >> 48) & 255);
	message[4] = (unsigned char)((data_length >> 40) & 255);
	message[5] = (unsigned char)((data_length >> 32) & 255);
	message[6] = (unsigned char)((data_length >> 24) & 255);
	message[7] = (unsigned char)((data_length >> 16) & 255);
	message[8] = (unsigned char)((data_length >> 8)  & 255);
	message[9] = (unsigned char)((data_length)       & 255);
	data_start_index = 10;
	memcpy(&message[data_start_index], data, data_length);
	return openssl_write_sync(connection, (char *)message, data_length + data_start_index);
}


int websocket_send_img(struct connection *connection, char *data, uint64_t data_length)
{
	unsigned char message[128 KB];
	int data_start_index;

	message[0] = 65;
	if(data_length <= 125) {
		message[1] = (unsigned char)data_length;
		data_start_index = 2;
	} else if( data_length > 125 && data_length <= 65535) {
		message[1] = 126;
		message[2] = (unsigned char)((data_length >> 8) & 255);
		message[3] = (unsigned char)((data_length) & 255);
		data_start_index = 4;
	} else {
		message[1] = 127;
		message[2] = (unsigned char)((data_length >> 56) & 255);
		message[3] = (unsigned char)((data_length >> 48) & 255);
		message[4] = (unsigned char)((data_length >> 40) & 255);
		message[5] = (unsigned char)((data_length >> 32) & 255);
		message[6] = (unsigned char)((data_length >> 24) & 255);
		message[7] = (unsigned char)((data_length >> 16) & 255);
		message[8] = (unsigned char)((data_length >> 8)  & 255);
		message[9] = (unsigned char)((data_length)       & 255);
		data_start_index = 10;
	}
	memcpy(&message[data_start_index], data, data_length);
	return openssl_write_sync(connection, (char *)message, data_length + data_start_index);
}

void websocket_send_gzip(struct connection *connection, char *data, uint64_t data_length)
{
	unsigned char message[256 KB];
	int data_start_index;

	message[0] = 131;
	message[0] = 0xc1;
	if (data_length <= 65535) {
		message[1] = 126;
		message[2] = (unsigned char)((data_length >> 8) & 255);
		message[3] = (unsigned char)((data_length) & 255);
		data_start_index = 4;
	} else {
		message[1] = 127;
		message[2] = (unsigned char)((data_length >> 56) & 255);
		message[3] = (unsigned char)((data_length >> 48) & 255);
		message[4] = (unsigned char)((data_length >> 40) & 255);
		message[5] = (unsigned char)((data_length >> 32) & 255);
		message[6] = (unsigned char)((data_length >> 24) & 255);
		message[7] = (unsigned char)((data_length >> 16) & 255);
		message[8] = (unsigned char)((data_length >> 8)  & 255);
		message[9] = (unsigned char)((data_length)       & 255);
		data_start_index = 10;
	}
	memcpy(&message[data_start_index], data, data_length);
	openssl_write_sync(connection, (char *)message, data_length + data_start_index);
}


int websocket_send2(struct connection *connection, char *data, uint64_t data_length)
{
	int data_start_index;

	data[0] = (unsigned char)129;
	data[1] = 126;
	data[2] = (unsigned char)((data_length >> 8) & 255);
	data[3] = (unsigned char)((data_length) & 255);
	data_start_index = 4;
	return openssl_write_sync(connection, (char *)data, data_length + data_start_index);
}

int websocket_send4(struct connection *connection, char *data, uint64_t data_length)
{
	int data_start_index;

	data[0] = (unsigned char)129;
	data[1] = 127;
	data[2] = (unsigned char)((data_length >> 56) & 255);
	data[3] = (unsigned char)((data_length >> 48) & 255);
	data[4] = (unsigned char)((data_length >> 40) & 255);
	data[5] = (unsigned char)((data_length >> 32) & 255);
	data[6] = (unsigned char)((data_length >> 24) & 255);
	data[7] = (unsigned char)((data_length >> 16) & 255);
	data[8] = (unsigned char)((data_length >> 8)  & 255);
	data[9] = (unsigned char)((data_length)       & 255);
	data_start_index = 10;
	return openssl_write_sync(connection, (char *)data, data_length + data_start_index);
}

int websocket_recv_deflate(char *data, uint64_t data_length, unsigned char *dst)
{
	return 0;
}
