/* extra/seose_compat.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "seose_compat.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

std::string seose_to_base62(std::uint16_t input)
{
	static char dict[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::string result;

	while (input > 0)
	{
		std::uint16_t x = input % 62;
		input /= 62;
		result = dict[x] + result;
	}

	return result;
}

std::uint16_t seose_hash(const char *input, std::size_t length, std::uint16_t method)
{
	int pow[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	std::uint16_t result = 0;

	for (std::size_t i = 0; i < length; ++i)
	{
		for (int j = 7; j >= 0; --j)
		{
			std::uint16_t test_bit = ((result & 0x8000) == 0x8000) ^ ((input[i] & pow[j]) == pow[j]);
			result = (result & 0x7FFF) * 2;

			if (test_bit)
				result = result ^ method;
		};
	}

	return result;
}

std::string seose_str_hash(const std::string& input, const std::string& key)
{
	std::string result;

	for (std::size_t i = 0; i < key.length(); ++i)
	{
		unsigned char kc = key[i];

		// Remind me to strangle Sordie for using pound symbols in the default key
		if (kc == '#')
			kc = char(0xA3); // pound character

		result += seose_to_base62(seose_hash(input.data(), input.length(), (i + 1) * kc));
	}

	return result;
}
