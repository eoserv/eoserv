
#include "config.hpp"

#include <cstdio>
#include <stdexcept>

#include "util.hpp"

Config::Config()
{

}

Config::Config(std::string filename)
{
	std::FILE *fh;
	char buf[Config::MaxLineLength];
	std::string line;
	std::string key;
	std::string val;
	std::size_t eqloc;

	this->filename = filename;

	fh = std::fopen(filename.c_str(), "rt");
	if (!fh)
	{
		throw std::runtime_error("Configuration file not found.");
	}

	while (!std::feof(fh))
	{
		if (std::fgets(buf, Config::MaxLineLength, fh))
		{
			line = util::trim(buf);
		}
		else
		{
			line = "";
		}

		if (line.length() < 1)
		{
			continue;
		}

		if (line[0] == '#')
		{
			continue;
		}

		eqloc = line.find('=');

		if (eqloc == std::string::npos)
		{
			continue;
		}

		key = util::rtrim(line.substr(0, eqloc));
		val = util::ltrim(line.substr(eqloc+1));

		this->insert(std::pair<std::string, util::variant>(key, static_cast<util::variant>(val)));
	}
}
