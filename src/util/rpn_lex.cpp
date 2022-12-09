/* util/rpn_lex.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "rpn_lex.hpp"

#include <cctype>
#include <functional>
#include <istream>
#include <limits>
#include <string>

namespace util
{
	static inline bool ctype(const char *chars, char c)
	{
		for (const char *p = chars; *p != '\0'; ++p)
		{
			if (*p == c)
				return true;
		}

		return false;
	}

	static bool ctype_op(char c) { return ctype("(),&|=!<>+-*/%&|!~", c); }
	static bool ctype_whitespace(char c) { return std::isspace(c); }
	static bool ctype_value(char c) { return !ctype_op(c) && !ctype_whitespace(c); }

	bool check_op(char a, char b)
	{
		switch (a)
		{
			case '(': case ')': case '+': return b == '\0';
			case '&': return b == '\0' || b == '&';
			case '|': return b == '\0' || b == '|';
			case '-': case '~': return b == '\0';
			case '*': case '/': case '%': return b == '\0';
			case '<': case '>': case '!': return b == '\0' || b == '=';
			case '=': return b == '\0' || b == '=';
			default: return false;
		}
	}

	Lexer::Lexer(std::istream& is)
		: is(is)
	{ }

	bool Lexer::PeekChar(char& c)
	{
		int cc = is.peek();

		if (cc == std::char_traits<char>::eof())
			return false;

		c = char(cc);
		return true;
	}

	bool Lexer::GetChar(char& c)
	{
		if (!is.get(c))
			return false;

		return true;
	}

	bool Lexer::GetCharIf(char& c, std::function<bool(char)> f)
	{
		char cc = '\0';

		if (!this->PeekChar(cc) || !f(cc))
			return false;

		return this->GetChar(c);
	}

	std::string Lexer::ReadValue()
	{
		char c = '\0';
		std::string s;

		this->GetChar(c);
		s += c;

		while (this->GetCharIf(c, ctype_value))
		{
			s += c;
		}

		return s;
	}

	std::string Lexer::ReadOperator()
	{
		char chars[3]{};

		this->GetChar(chars[0]);
		this->PeekChar(chars[1]);

		if (check_op(chars[0], chars[1]))
			this->GetChar(chars[1]);
		else
			chars[1] = '\0';

		return chars;
	}

	std::string Lexer::ReadToken()
	{
		char c;

		while (this->GetCharIf(c, ctype_whitespace)) ; // no loop body

		if (!this->token_buffer.empty())
		{
			std::string token = this->token_buffer.front();
			this->token_buffer.pop();
			return token;
		}

		if (this->PeekChar(c))
		{
			if (ctype_op(c))
			{
				return this->ReadOperator();
			}
			else
			{
				return this->ReadValue();
			}
		}

		return {};
	}
}
