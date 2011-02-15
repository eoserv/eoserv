
/* sha256.c
 * Copyright 2009 the EOSERV development team (http://eoserv.net/devs)
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

#define PACK_U32(dest, src) \
(dest) = ((u32)(src)[0] << 24) | \
		 ((u32)(src)[1] << 16) | \
		 ((u32)(src)[2] << 8 ) | \
		 ((u32)(src)[3]      );

#define UNPACK_U32(dest, src) \
(dest)[0] = (u8)((src) >> 24); \
(dest)[1] = (u8)((src) >> 16); \
(dest)[2] = (u8)((src) >> 8 ); \
(dest)[3] = (u8)((src)      );

#define RR(x, y) \
( ((u32)((x) & u32_max) >> (y)) | ((u32)((x) & u32_max) << (32 - (y))) )

#define BLOCK_SIZE 64
#define HASH_SIZE 8

static const u32 k[BLOCK_SIZE] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B,
	0x59F111F1, 0x923F82A4, 0xAB1C5ED5, 0xD807AA98, 0x12835B01,
	0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7,
	0xC19BF174, 0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA, 0x983E5152,
	0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147,
	0x06CA6351, 0x14292967, 0x27B70A85, 0x2E1B2138, 0x4D2C6DFC,
	0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819,
	0xD6990624, 0xF40E3585, 0x106AA070, 0x19A4C116, 0x1E376C08,
	0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F,
	0x682E6FF3, 0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static const u32 h[HASH_SIZE] = {
#ifndef SHA224
	0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
	0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
#else // SHA224
	0xC1059ED8, 0x367CD507, 0x3070DD17, 0xF70E5939,
	0xFFC00B31, 0x68581511, 0x64F98FA7, 0xBEFA4FA4
#endif // SHA224
};

inline void sha256_memcpy_8(u8 *dest, const u8 *src, unsigned long count)
{
	unsigned long i = 0;

	for (; i < count; ++i)
	{
		dest[i] = src[i];
	}
}

void sha256_start(sha256_context *ctx)
{
#ifndef SHA256_FAST
	int i;
	for (i = 0; i < HASH_SIZE; ++i)
	{
		ctx->h[i] = h[i];
	}
#else // SHA256_FAST
	ctx->h[0] = h[0]; ctx->h[1] = h[1];
	ctx->h[2] = h[2]; ctx->h[3] = h[3];
	ctx->h[4] = h[4]; ctx->h[5] = h[5];
	ctx->h[6] = h[6]; ctx->h[7] = h[7];
#endif // SHA256_FAST

	ctx->length[0] = 0;
	ctx->length[1] = 0;
}

#define F1(i) \
w[i] = w[i-16] + (RR(w[i-15], 7) ^ RR(w[i-15], 18) ^ (w[i-15] >> 3)) + \
w[i-7] + (RR(w[i-2], 17) ^ RR(w[i-2], 19) ^ (w[i-2] >> 10))

#define R1(i) PACK_U32(w[i], data + (i << 2))

#define R2(a,b,c,d,e,f,g,h,i) \
d += t = h + (RR(e, 6) ^ RR(e, 11) ^ RR(e, 25)) + \
(g ^ (e & (f ^ g))) + k[i] + (i < 16 ? w[i] : (F1(i))); \
h = t + (RR(a, 2) ^ RR(a, 13) ^ RR(a, 22)) + \
((a & b) | (c & (a | b)));

static void sha256_process(sha256_context *ctx, const u8 data[BLOCK_SIZE])
{
#ifndef SHA256_FAST
	int i;
#endif // SHA256_FAST
	u32 t, w[BLOCK_SIZE];
#ifndef SHA256_FAST
	u32 s[HASH_SIZE];
#else // SHA256_FAST
	u32 a, b, c, d, e, f, g, h;
#endif // SHA256_FAST

#ifndef SHA256_FAST
	for (i = 0; i < HASH_SIZE; ++i)
	{
		s[i] = ctx->h[i];
	}
#else // SHA256_FAST
	a = ctx->h[0]; b = ctx->h[1];
	c = ctx->h[2]; d = ctx->h[3];
	e = ctx->h[4]; f = ctx->h[5];
	g = ctx->h[6]; h = ctx->h[7];
#endif // SHA256_FAST

#ifndef SHA256_FAST
	for (i = 0; i < 16; ++i)
	{
		PACK_U32(w[i], data + (i << 2))
	}
#else // SHA256_FAST
	R1(0) R1(1) R1(2)  R1(3)  R1(4)  R1(5)  R1(6)  R1(7)
	R1(8) R1(9) R1(10) R1(11) R1(12) R1(13) R1(14) R1(15)
#endif // SHA256_FAST

#ifndef SHA256_FAST
	for (i = 0; i < 64; ++i)
	{
		t = s[7] + (RR(s[4], 6) ^ RR(s[4], 11) ^ RR(s[4], 25)) + (s[6] ^ (s[4] & (s[5] ^ s[6]))) + \
		k[i] + (i < 16 ? w[i] : (F1(i)));
		s[7] = s[6]; s[6] = s[5]; s[5] = s[4];
		s[4] = s[3] + t;
		s[3] = s[2]; s[2] = s[1]; s[1] = s[0];
		s[0] = t + (RR(s[1], 2) ^ RR(s[1], 13) ^ RR(s[1], 22)) + ((s[1] & s[2]) | (s[3] & (s[1] | s[2])));
	}
#else // SHA256_FAST
	R2(a, b, c, d, e, f, g, h, 0)  R2(h, a, b, c, d, e, f, g, 1)
	R2(g, h, a, b, c, d, e, f, 2)  R2(f, g, h, a, b, c, d, e, 3)
	R2(e, f, g, h, a, b, c, d, 4)  R2(d, e, f, g, h, a, b, c, 5)
	R2(c, d, e, f, g, h, a, b, 6)  R2(b, c, d, e, f, g, h, a, 7)
	R2(a, b, c, d, e, f, g, h, 8)  R2(h, a, b, c, d, e, f, g, 9)
	R2(g, h, a, b, c, d, e, f, 10) R2(f, g, h, a, b, c, d, e, 11)
	R2(e, f, g, h, a, b, c, d, 12) R2(d, e, f, g, h, a, b, c, 13)
	R2(c, d, e, f, g, h, a, b, 14) R2(b, c, d, e, f, g, h, a, 15)
	R2(a, b, c, d, e, f, g, h, 16) R2(h, a, b, c, d, e, f, g, 17)
	R2(g, h, a, b, c, d, e, f, 18) R2(f, g, h, a, b, c, d, e, 19)
	R2(e, f, g, h, a, b, c, d, 20) R2(d, e, f, g, h, a, b, c, 21)
	R2(c, d, e, f, g, h, a, b, 22) R2(b, c, d, e, f, g, h, a, 23)
	R2(a, b, c, d, e, f, g, h, 24) R2(h, a, b, c, d, e, f, g, 25)
	R2(g, h, a, b, c, d, e, f, 26) R2(f, g, h, a, b, c, d, e, 27)
	R2(e, f, g, h, a, b, c, d, 28) R2(d, e, f, g, h, a, b, c, 29)
	R2(c, d, e, f, g, h, a, b, 30) R2(b, c, d, e, f, g, h, a, 31)
	R2(a, b, c, d, e, f, g, h, 32) R2(h, a, b, c, d, e, f, g, 33)
	R2(g, h, a, b, c, d, e, f, 34) R2(f, g, h, a, b, c, d, e, 35)
	R2(e, f, g, h, a, b, c, d, 36) R2(d, e, f, g, h, a, b, c, 37)
	R2(c, d, e, f, g, h, a, b, 38) R2(b, c, d, e, f, g, h, a, 39)
	R2(a, b, c, d, e, f, g, h, 40) R2(h, a, b, c, d, e, f, g, 41)
	R2(g, h, a, b, c, d, e, f, 42) R2(f, g, h, a, b, c, d, e, 43)
	R2(e, f, g, h, a, b, c, d, 44) R2(d, e, f, g, h, a, b, c, 45)
	R2(c, d, e, f, g, h, a, b, 46) R2(b, c, d, e, f, g, h, a, 47)
	R2(a, b, c, d, e, f, g, h, 48) R2(h, a, b, c, d, e, f, g, 49)
	R2(g, h, a, b, c, d, e, f, 50) R2(f, g, h, a, b, c, d, e, 51)
	R2(e, f, g, h, a, b, c, d, 52) R2(d, e, f, g, h, a, b, c, 53)
	R2(c, d, e, f, g, h, a, b, 54) R2(b, c, d, e, f, g, h, a, 55)
	R2(a, b, c, d, e, f, g, h, 56) R2(h, a, b, c, d, e, f, g, 57)
	R2(g, h, a, b, c, d, e, f, 58) R2(f, g, h, a, b, c, d, e, 59)
	R2(e, f, g, h, a, b, c, d, 60) R2(d, e, f, g, h, a, b, c, 61)
	R2(c, d, e, f, g, h, a, b, 62) R2(b, c, d, e, f, g, h, a, 63)
#endif // SHA256_FAST

#ifndef SHA256_FAST
	for (i = 0; i < 8; ++i)
	{
		ctx->h[i] += s[i];
	}
#else // SHA256_FAST
	ctx->h[0] += a; ctx->h[1] += b;
	ctx->h[2] += c; ctx->h[3] += d;
	ctx->h[4] += e; ctx->h[5] += f;
	ctx->h[6] += g; ctx->h[7] += h;
#endif // SHA256_FAST
}

void sha256_update(sha256_context *ctx, const u8 *input, unsigned long length)
{
	unsigned long n;
	int pos = ctx->length[0] & 0x3F;

	ctx->length[0] += length;
	ctx->length[0] &= u32_max;

	if (ctx->length[0] < length)
	{
		++ctx->length[1];
	}

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
			n = (length < (BLOCK_SIZE - pos)) ? length : (BLOCK_SIZE - pos);
			sha256_memcpy_8(ctx->buf + pos, input, n);
			pos += n;
			input += n;
			length -= n;

			if (pos == BLOCK_SIZE)
			{
				sha256_process(ctx, ctx->buf);
			}
		}
	}
}

void sha256_finish(sha256_context *ctx, u8 digest[HASH_SIZE * 4])
{
#ifndef SHA256_FAST
	int i;
#endif // SHA256_FAST
	u8 length[8];
	u32 h = (ctx->length[0] >> 29) | (ctx->length[1] << 3);
	u32 l = ctx->length[0] << 3;
	int pos = ctx->length[0] & 0x3F;

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

	UNPACK_U32(length, h)
	UNPACK_U32(length + 4, l)

	sha256_memcpy_8(ctx->buf + pos, length, 8);

	sha256_process(ctx, ctx->buf);

#ifndef SHA224
#ifndef SHA256_FAST
	for (i = 0; i < 8; ++i)
	{
		UNPACK_U32(digest + i * 4, ctx->h[i])
	}
#else // SHA256_FAST
	UNPACK_U32(digest     , ctx->h[0])
	UNPACK_U32(digest +  4, ctx->h[1])
	UNPACK_U32(digest +  8, ctx->h[2])
	UNPACK_U32(digest + 12, ctx->h[3])
	UNPACK_U32(digest + 16, ctx->h[4])
	UNPACK_U32(digest + 20, ctx->h[5])
	UNPACK_U32(digest + 24, ctx->h[6])
	UNPACK_U32(digest + 28, ctx->h[7])
#endif // SHA256_FAST
#else // SHA224
	UNPACK_U32(digest, 0)
#ifndef SHA256_FAST
	for (i = 1; i < 8; ++i)
	{
		UNPACK_U32(digest + i * 4, ctx->h[i - 1])
	}
#else // SHA256_FAST
	UNPACK_U32(digest +  4, ctx->h[0])
	UNPACK_U32(digest +  8, ctx->h[1])
	UNPACK_U32(digest + 12, ctx->h[2])
	UNPACK_U32(digest + 16, ctx->h[3])
	UNPACK_U32(digest + 20, ctx->h[4])
	UNPACK_U32(digest + 24, ctx->h[5])
	UNPACK_U32(digest + 28, ctx->h[6])
#endif // SHA256_FAST
#endif // SHA224
}
