/* extra/seose_compat.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EXTRA_SEOSE_COMPAT_HPP_INCLUDED
#define EXTRA_SEOSE_COMPAT_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>

std::string seose_to_base62(std::uint16_t input);
std::uint16_t seose_hash(const char *input, std::size_t length, std::uint16_t method);
std::string seose_str_hash(const std::string& input, const std::string& key);

#endif // EXTRA_SEOSE_COMPAT_HPP_INCLUDED
