#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <string>
#include <map>

#include "variant.hpp"

class Config;

class Config : public std::map<std::string, variant>
{
	protected:
		std::string filename;

	public:
		static const int MaxLineLength = 4096;
		Config(std::string filename);
};

#endif // CONFIG_HPP_INCLUDED
