
#include "config.hpp"

#include <cstdio>
#include <stdexcept>

#include "util.hpp"
#include "variant.hpp"

Config::Config(std::string filename)
{
	FILE *fh;
	char buf[Config::MaxLineLength];
	std::string line;
	std::string key;
	std::string val;
	size_t eqloc;

	this->filename = filename;

	fh = fopen(filename.c_str(), "rt");
	if (!fh)
	{
		throw std::runtime_error("Configuration file not found.");
	}

	while (!feof(fh))
	{
		if (fgets(buf, Config::MaxLineLength, fh))
		{
			line = trim(buf);
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

		key = rtrim(line.substr(0, eqloc));
		val = ltrim(line.substr(eqloc+1));

		this->insert(std::pair<std::string, variant>(key, static_cast<variant>(val)));
	}
}
