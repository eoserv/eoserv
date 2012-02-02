
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef I18N_HPP_INCLUDED
#define I18N_HPP_INCLUDED

#include <memory>
#include <utility>
#include <vector>

#include "fwd/config.hpp"

#include "util/variant.hpp"

class I18N
{
	protected:
		std::unique_ptr<Config> lang_config;

	public:
		I18N();
		I18N(std::string lang_file);

		void SetLangFile(std::string lang_file);

		std::string FormatV(std::string id, std::vector<util::variant> &&v);

		template <class... Args> std::string Format(std::string id, Args&&... args)
		{
			std::vector<util::variant> v{args...};
			return FormatV(id, std::move(v));
		}

		~I18N();
};

#endif // I18N_HPP_INCLUDED
