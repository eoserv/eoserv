#ifndef HASH_HPP_INCLUDED
#define HASH_HPP_INCLUDED

extern "C"
{
#include "sha256.h"
}

#include <string>

#ifdef uint8
#undef uint8
#endif // uint8

#ifdef uint32
#undef uint32
#endif // uint32

void sha256(std::string &);

#endif // HASH_HPP_INCLUDED
