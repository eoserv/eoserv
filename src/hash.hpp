
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HASH_HPP_INCLUDED
#define HASH_HPP_INCLUDED

#include <string>

#include "script.hpp"

/**
 * Convert a string to the hex representation of it's sha256 hash
 */
std::string sha256(std::string);

namespace hash
{

inline void ScriptRegister(ScriptEngine &engine)
{
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string sha256(string)", sha256);
}

}

#endif // HASH_HPP_INCLUDED
