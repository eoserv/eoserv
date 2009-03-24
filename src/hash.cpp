
#include "hash.hpp"

#include <string>
#include <cstring>

extern "C"
{
#include "sha256.h"
}

void sha256(std::string &str)
{
	sha256_context ctx;
	unsigned char *data = new unsigned char[str.length()];
	unsigned char digest[32];
	char cdigest[64];
	std::memcpy(static_cast<void *>(data), static_cast<const void *>(str.c_str()), str.length());

	sha256_starts(&ctx);
	sha256_update(&ctx, data, str.length());
	sha256_finish(&ctx, digest);

	for (int i = 0; i <= 32; ++i)
	{
		cdigest[i*2]   = "0123456789abcdef"[((digest[i] >> 4) & 0x0F)];
		cdigest[i*2+1] = "0123456789abcdef"[((digest[i]) & 0x0F)];
	}

	str.assign(cdigest, 64);

	delete data;
}
