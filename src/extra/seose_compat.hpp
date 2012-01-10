
#include <string>
#include <cstdint>

std::string seose_to_base62(std::uint16_t input);
std::uint16_t seose_hash(const char *input, std::size_t length, std::uint16_t method);
std::string seose_str_hash(const std::string& input, const std::string& key);
