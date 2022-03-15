/* util/secure_string.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_SECURE_STRING_HPP_INCLUDED
#define UTIL_SECURE_STRING_HPP_INCLUDED

#include "../util.hpp"

#include <algorithm>
#include <string>

namespace util
{

/**
 * A string wrapper which automatically nulls out it's memory buffer when
 * destroyed or overwritten
 */
struct secure_string
{
	private:
		std::string str_;

	public:
		secure_string(std::string&& s)
			: str_(s)
		{
			std::fill(UTIL_RANGE(s), '\0');
			s.erase();
		}

		secure_string(const secure_string& other)
		{
			erase();
			this->str_ = other.str_;
		}

		secure_string& operator =(const secure_string& rhs)
		{
			erase();
			this->str_ = rhs.str_;
			return *this;
		}

		secure_string& operator =(secure_string&& rhs)
		{
			erase();
			this->str_ = rhs.str_;
			rhs.erase();
			return *this;
		}

		secure_string& operator =(std::string&& rhs)
		{
			erase();
			this->str_ = rhs;
			std::fill(UTIL_RANGE(rhs), '\0');
			rhs.erase();
			return *this;
		}

		void erase()
		{
			std::fill(UTIL_RANGE(this->str_), '\0');
			this->str_.erase();
		}

		/// Make sure nothing copies or mutates this string!
		const std::string& str()
		{
			return this->str_;
		}

		~secure_string()
		{
			erase();
		}
};

}

#endif // UTIL_SECURE_STRING_HPP_INCLUDED
