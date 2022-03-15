/* sha256.c
 * Copyright (c) Julian Smythe, All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "sha256.h"

#include <string.h>

#define BLOCK_SIZE SHA256_BLOCK_SIZE
#define HASH_SIZE  SHA256_HASH_SIZE

static const uint32_t k[BLOCK_SIZE] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static const uint32_t h[HASH_SIZE] = {
	0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
	0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

static inline uint32_t pack_u32(const char *src)
{
	return ((uint32_t)(unsigned char)src[0] << 24) |
	       ((uint32_t)(unsigned char)src[1] << 16) |
	       ((uint32_t)(unsigned char)src[2] <<  8) |
	       ((uint32_t)(unsigned char)src[3]      );
}

static inline void unpack_u32(char *dest, uint32_t src)
{
	dest[0] = (char)(unsigned char)(src >> 24);
	dest[1] = (char)(unsigned char)(src >> 16);
	dest[2] = (char)(unsigned char)(src >>  8);
	dest[3] = (char)(unsigned char)(src      );
}

static inline uint32_t rr32(uint32_t x, int y)
{
	return (x >> y) | (x << (32 - y));
}

void sha256_start(sha256_context *ctx)
{
	size_t i;

	for (i = 0; i < HASH_SIZE; ++i)
	{
		ctx->h[i] = h[i];
	}

	ctx->length = 0;
}

static void sha256_process(sha256_context *ctx, const char data[BLOCK_SIZE])
{
	size_t i;
	uint32_t t;
	uint32_t w[BLOCK_SIZE];
	uint32_t s[HASH_SIZE];

	for (i = 0; i < HASH_SIZE; ++i)
	{
		s[i] = ctx->h[i];
	}

	for (i = 0; i < 16; ++i)
	{
		w[i] = pack_u32(data + (i << 2));
	}

	for (; i < 64; ++i)
	{
		w[i] = w[i - 16] + (rr32(w[i - 15],  7) ^ rr32(w[i - 15], 18) ^ (w[i - 15] >>  3))
		     + w[i -  7] + (rr32(w[i -  2], 17) ^ rr32(w[i -  2], 19) ^ (w[i -  2] >> 10));
	}

	for (i = 0; i < 64; ++i)
	{
		t = s[7] + (rr32(s[4], 6) ^ rr32(s[4], 11) ^ rr32(s[4], 25)) + (s[6] ^ (s[4] & (s[5] ^ s[6]))) + k[i] + w[i];
		s[7] = s[6];
		s[6] = s[5];
		s[5] = s[4];
		s[4] = s[3] + t;
		s[3] = s[2];
		s[2] = s[1];
		s[1] = s[0];
		s[0] = t + (rr32(s[1], 2) ^ rr32(s[1], 13) ^ rr32(s[1], 22)) + ((s[1] & s[2]) | (s[3] & (s[1] | s[2])));
	}

	for (i = 0; i < 8; ++i)
	{
		ctx->h[i] += s[i];
	}
}

void sha256_update(sha256_context *ctx, const char *input, uint64_t length)
{
	// SHA256_BLOCK_SIZE is a power of two
	size_t pos = (size_t)(ctx->length & (SHA256_BLOCK_SIZE - 1));

	ctx->length += length;

	while (length > 0)
	{
		if (length >= BLOCK_SIZE && pos == 0)
		{
			sha256_process(ctx, input);
			input += BLOCK_SIZE;
			length -= BLOCK_SIZE;
		}
		else
		{
			size_t n = (length < (BLOCK_SIZE - pos)) ? (size_t)length : (size_t)(BLOCK_SIZE - pos);
			memcpy(ctx->buf + pos, input, n);
			pos += n;
			input += n;
			length -= n;

			if (pos == BLOCK_SIZE)
			{
				sha256_process(ctx, ctx->buf);
				pos = 0;
			}
		}
	}
}

void sha256_finish(sha256_context *ctx, char digest[HASH_SIZE * 4])
{
	char length[8];
	uint32_t h = (uint32_t)(ctx->length >> 29); // Truncation
	uint32_t l = (uint32_t)(ctx->length <<  3); // Truncation
	size_t pos = (size_t)(ctx->length & 0x3F);

	ctx->buf[pos++] = 0x80;

	if (pos > BLOCK_SIZE - 8)
	{
		while (pos < BLOCK_SIZE)
		{
			ctx->buf[pos++] = 0;
		}

		sha256_process(ctx, ctx->buf);
		pos = 0;
	}

	while (pos < BLOCK_SIZE - 8)
	{
		ctx->buf[pos++] = 0;
	}

	unpack_u32(length, h);
	unpack_u32(length + 4, l);

	memcpy(ctx->buf + pos, length, 8);

	sha256_process(ctx, ctx->buf);

	if (digest)
	{
		size_t i;

		for (i = 0; i < 8; ++i)
		{
			unpack_u32((char *)(digest + i * 4), ctx->h[i]);
		}
	}
}
