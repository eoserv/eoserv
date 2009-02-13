#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>

struct pairchar
{
	unsigned char byte[2];

	unsigned char &operator[](int);
};

struct quadchar
{
	unsigned char byte[4];

	unsigned char &operator[](int);
};

std::string ltrim(const std::string &);
std::string rtrim(const std::string &);
std::string trim(const std::string &);

#endif // UTIL_HPP_INCLUDED
