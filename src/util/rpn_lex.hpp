/* util/rpn_lex.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_RPN_LEX_HPP_INCLUDED
#define UTIL_RPN_LEX_HPP_INCLUDED

#include <functional>
#include <istream>
#include <queue>
#include <string>

namespace util
{
	class Lexer
	{
		private:
			std::basic_istream<char>& is;
			std::queue<std::string> token_buffer;

			bool PeekChar(char& c);
			bool GetChar(char& c);
			bool GetCharIf(char& c, std::function<bool(char)> f);

			std::string ReadValue();
			std::string ReadOperator();

		public:
			Lexer(std::basic_istream<char>& is);

			std::string ReadToken();

			template <class IT> IT Lex(IT it)
			{
				std::string t;

				for (t = ReadToken(); !t.empty(); t = ReadToken())
					*it++ = t;

				return it;
			}
	};
}

#endif // UTIL_RPN_LEX_HPP_INCLUDED
