#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <string>
#include <map>
#include <cstddef>

#include "variant.hpp"

class Config;

class Config : public std::map<std::string, variant>
{
	protected:
		std::string filename;

	public:
		static const std::size_t MaxLineLength = 4096;
		Config(std::string filename);
};

#endif // CONFIG_HPP_INCLUDED
