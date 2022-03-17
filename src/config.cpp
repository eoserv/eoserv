/* config.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "config.hpp"

#include "console.hpp"
#include "util.hpp"
#include "util/variant.hpp"

#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include <string>

bool Config::Read(const std::string& filename, bool nowarn)
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
		if (!nowarn)
			Console::Wrn("Configuration file not found: %s", filename.c_str());
		return false;
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

		if (line.length() > eqloc+1)
		{
			val = util::ltrim(line.substr(eqloc+1));
		}
		else
		{
			val = std::string("");
		}

		if (key == "REQUIRE")
		{
			if (!this->Read(val))
				return false;
		}
		else if (key == "INCLUDE" || key == "INCLUDE_NOWARN")
		{
			bool nowarn = (key == "INCLUDE_NOWARN");
			this->Read(val, nowarn);
		}
		else
		{
			std::size_t loc = val.find('\\');
			while (loc != std::string::npos && loc != val.length())
			{
				if (val[loc + 1] == 't')
				{
					val.replace(loc, 2, "\t");
				}
				else if (val[loc + 1] == 'r')
				{
					val.replace(loc, 2, "\r");
				}
				else if (val[loc + 1] == 'n')
				{
					val.replace(loc, 2, "\n");
				}
				else if (val[loc + 1] == '\\')
				{
					val.replace(loc, 2, "\\");
				}

				loc = val.find('\\', loc+1);
			}

			this->operator[](key) = static_cast<util::variant>(val);
		}
	}

	std::fclose(fh);
	return true;
}
