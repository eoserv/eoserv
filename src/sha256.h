/* sha256.h
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

#ifndef SHA256_H_INCLUDED
#define SHA256_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define SHA256_BLOCK_SIZE 64
#define SHA256_HASH_SIZE 8

typedef struct sha256_context
{
	uint32_t h[SHA256_HASH_SIZE];
	char buf[SHA256_BLOCK_SIZE];
	uint64_t length;
} sha256_context;

void sha256_start(sha256_context *ctx);
void sha256_update(sha256_context *ctx, const char *input, uint64_t length);
void sha256_finish(sha256_context *ctx, char digest[SHA256_HASH_SIZE * 4]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SHA256_H_INCLUDED */
