/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 */

/* from https://github.com/lpereira/lwan/blob/master/src/lib/base64.c
	- changes for separate dyn/stack conversions
	- removed header file
 */

#include <stdinc.h>

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * Calculated using:
 * memset(dtable, 0x80, 256);
 * for (i = 0; i < sizeof(base64_table) - 1; i++)
 *    dtable[base64_table[i]] = (unsigned char) i;
 * dtable['='] = 0;
 */
static const unsigned char base64_decode_table[256] = {
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3e, 0x80, 0x80, 0x80, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x80, 0x80,
    0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80};

static inline size_t base64_encoded_len(size_t decoded_len)
{
    /* This counts the padding bytes (by rounding to the next multiple of 4). */
    return ((4u * decoded_len / 3u) + 3u) & ~3u;
}


bool
base64_validate(unsigned char *src, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (base64_decode_table[src[i]] == 0x80)
            return false;
    }
    return true;
}

size_t
base64_encode(unsigned char *src, size_t len, unsigned char *out)
{
    unsigned char *pos;
    unsigned char *end, *in;
    size_t         olen;

    olen = base64_encoded_len(len) + 1 /* for NUL termination */;
    if (olen < len)
        return 0; /* integer overflow */

    end = src + len;
    in  = src;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2]   & 0x3f];
        in    += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }
    *pos = '\0';
    return (pos-out);
}

char *base64_encode_malloc(unsigned char *src, size_t *len)
{
	return NULL;
}

unsigned char *base64_decode_malloc(const unsigned char *src, size_t len, size_t *out_len)
{
    unsigned char *out, *pos, block[4];
    size_t i, count, olen;
    int pad = 0;

    count = 0;
    for (i = 0; i < len; i++) {
        if (base64_decode_table[src[i]] != 0x80)
            count++;
    }

    if (count == 0 || count % 4)
        return NULL;

    olen = (count / 4 * 3) + 1;
    pos = out = malloc(olen);
    if (out == NULL)
        return NULL;

    count = 0;
    for (i = 0; i < len; i++) {
        unsigned char tmp = base64_decode_table[src[i]];
        if (tmp == 0x80)
            continue;

        if (src[i] == '=')
            pad++;
        block[count] = tmp;
        count++;
        if (count == 4) {
            *pos++ = (unsigned char)((block[0] << 2) | (block[1] >> 4));
            *pos++ = (unsigned char)((block[1] << 4) | (block[2] >> 2));
            *pos++ = (unsigned char)((block[2] << 6) | block[3]);
            count = 0;
            if (pad) {
                if (pad == 1)
                    pos--;
                else if (pad == 2)
                    pos -= 2;
                else {
                    /* Invalid padding */
                    free(out);
                    return NULL;
                }
                break;
            }
        }
    }
    *pos = '\0';

    *out_len = (size_t)(pos - out);
    return out;
}

size_t
base64_decode(unsigned char *src, size_t len, unsigned char *out)
{
    unsigned char *pos, block[4];
    size_t         olen, count = 0;
    int            pad = 0;

    for (int i = 0; i < len; i++)
        if (base64_decode_table[src[i]] != 0x80)
            count++;

    if (count == 0 || count % 4)
        return 0;

    olen  = (count / 4 * 3) + 1;
	pos   = out;
    count = 0;
    for (int i = 0; i < len; i++) {
        unsigned char tmp = base64_decode_table[src[i]];
        if (tmp == 0x80)
            continue;

        if (src[i] == '=')
            pad++;
        block[count] = tmp;
        count++;
        if (count == 4) {
            *pos++ = (unsigned char)((block[0] << 2) | (block[1] >> 4));
            *pos++ = (unsigned char)((block[1] << 4) | (block[2] >> 2));
            *pos++ = (unsigned char)((block[2] << 6) | block[3]);
            count = 0;
            if (pad) {
                if (pad == 1)
                    pos--;
                else if (pad == 2)
                    pos -= 2;
                else {
                    /* Invalid padding */
                    *out = 0;
                    return 0;
                }
                break;
            }
        }
    }
    *pos = '\0';
    return (pos-out);
}
