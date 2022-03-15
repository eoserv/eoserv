/* i18n.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "i18n.hpp"

#include "config.hpp"

#include "util.hpp"

#include <cstddef>
#include <string>
#include <vector>

I18N::I18N()
{ }

I18N::I18N(const std::string& lang_file)
	: lang_config(new Config(lang_file))
{ }

void I18N::SetLangFile(const std::string& lang_file)
{
	lang_config->Read(lang_file);
}

std::string I18N::FormatV(const std::string& id, std::vector<util::variant> &&v) const
{
	auto it = lang_config->find(id);

	if (it == lang_config->end())
	{
		std::string result = id;

		UTIL_FOREACH(v, x)
		{
			result += " " + std::string(x);
		}

		return result;
	}

	std::string result;
	std::string format = std::string(it->second);
	std::string number_buffer;
	int state = 0;

	UTIL_FOREACH(format, c)
	{
		if (state == 0)
		{
			if (c == '{')
			{
				state = 1;
			}
			else
			{
				result += c;
			}
		}
		else if (state == 1)
		{
			if (c == '}')
			{
				std::size_t index = int(util::variant(number_buffer)) - 1;

				if (index >= v.size())
					result += "#ERROR#";
				else
					result += std::string(v[index]);

				number_buffer = "";
				state = 0;
			}
			else
			{
				number_buffer += c;
			}
		}
	}

	return result;
}

I18N::~I18N()
{ }
