/* i18n.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef I18N_HPP_INCLUDED
#define I18N_HPP_INCLUDED

#include "fwd/i18n.hpp"

#include "fwd/config.hpp"

#include "util/variant.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

class I18N
{
	protected:
		std::unique_ptr<Config> lang_config;

	public:
		I18N();
		I18N(const std::string& lang_file);

		void SetLangFile(const std::string& lang_file);

		std::string FormatV(const std::string& id, std::vector<util::variant> &&v) const;

		template <class... Args> std::string Format(const std::string& id, Args&&... args) const
		{
			std::vector<util::variant> v{args...};
			return FormatV(id, std::move(v));
		}

		~I18N();
};

#endif // I18N_HPP_INCLUDED
