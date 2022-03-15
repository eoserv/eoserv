/* eoplus/lex.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "lex.hpp"

#include "../util.hpp"

#include <cctype>
#include <functional>
#include <istream>
#include <limits>
#include <string>

namespace EOPlus
{
	inline bool ctype(const char *chars, char c)
	{
		for (const char *p = chars; *p != '\0'; ++p)
		{
			if (*p == c)
				return true;
		}

		return false;
	}

	bool ctype_op(char c) { return ctype("()&|=!<>+-*/%&|!", c); }
	bool ctype_symbol(char c) { return ctype(";{}.,", c); }
	bool ctype_whitespace(char c) { return std::isspace(c); }
	bool ctype_alpha(char c) { return std::isalpha(c); }
	bool ctype_digit(char c) { return std::isdigit(c); }
	bool ctype_alnum(char c) { return ctype_alpha(c) || ctype_digit(c); }
	bool ctype_ident_start(char c) { return ctype_alpha(c) || c == '_' || c == '$'; }
	bool ctype_ident(char c) { return c != '$' && (ctype_ident_start(c) || ctype_digit(c)); }

	bool check_op(char a, char b)
	{
		switch (a)
		{
			case '(': case ')': case '+': return b == '\0';
			case '&': return b == '\0' || b == '&';
			case '|': return b == '\0' || b == '|';
			case '-': return b == '\0' || b == UOP_ALT;
			case '*': case '/': case '%': return b == '\0';
			case '<': case '>': case '!': return b == '\0' || b == '=';
			case '=': return b == '=';
			default: return false;
		}
	}

	Lexer::Lexer(std::istream& is)
		: row(1)
		, col(0)
		, is(is)
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

		++this->col;

		if (c == '\n')
		{
			this->token_buffer.push(Token(Token::NewLine));
			++this->row;
			this->col = 0;
		}

		return true;
	}

	bool Lexer::GetCharIf(char& c, std::function<bool(char)> f)
	{
		char cc = '\0';

		if (!this->PeekChar(cc) || !f(cc))
			return false;

		return this->GetChar(c);
	}

	Token Lexer::ReadNumber()
	{
		char c = '\0';

		if (!this->GetCharIf(c, ctype_digit))
			return Token();

		int number = (c - '0');

		while (this->GetCharIf(c, ctype_digit))
		{
			if (number > std::numeric_limits<int>::max() / 10)
				number = std::numeric_limits<int>::max();

			number *= 10;
			number += (c - '0');
		}

		return Token(Token::Integer, number);
	}

	Token Lexer::ReadString()
	{
		char c = '\0';
		std::string s;
		bool escape = false;

		if (!this->GetCharIf(c, [](char c) { return c == '"'; }))
			return Token();

		while (this->GetChar(c))
		{
			if (escape)
			{
				switch (c)
				{
					// No real need for any explicit escape codes...
					default: s += c; break;
				};

				escape = false;
			}
			else if (c == '\\')
			{
				escape = true;
			}
			else if (c == '"')
			{
				break;
			}
			else if (c == '\n')
			{
				throw Lexer_Error("Unterminated string on previous line", this->row, this->col);
			}
			else
			{
				s += c;
			}
		}

		return Token(Token::String, s);
	}

	Token Lexer::ReadIdentifier()
	{
		char c = '\0';
		std::string s;

		if (!this->GetCharIf(c, ctype_ident_start))
			return Token();

		s += c;

		while (this->GetCharIf(c, ctype_ident))
		{
			s += c;
		}

		s = util::lowercase(s);

		if (s == "true" || s == "false")
		{
			return Token(Token::Boolean, s == "true" ? true : false);
		}
		else
		{
			return Token(Token::Identifier, s);
		}
	}

	Token Lexer::ReadSymbol()
	{
		char c = '\0';

		if (!this->GetCharIf(c, ctype_symbol))
			return Token();

		return Token(Token::Symbol, std::string(1, c));
	}

	Token Lexer::ReadOperator()
	{
		char c = '\0';
		char cc = '\0';

		if (!this->GetCharIf(c, ctype_op))
			return Token();

		this->PeekChar(cc);

		if (check_op(c, cc))
			this->GetCharIf(cc, ctype_op);

		if (check_op(c, cc))
			return Token(Token::Operator, UOP2(c, cc));
		else if (check_op(c, '\0'))
			return Token(Token::Operator, UOP2(c, '\0'));
		else
			return Token();
	}

	Token Lexer::ReadToken()
	{
		Token token;

		char c = '\0';

		while (this->GetCharIf(c, ctype_whitespace)) ; // no loop body

		if (!this->token_buffer.empty())
		{
			token = this->token_buffer.front();
			this->token_buffer.pop();
			return token;
		}

		if (this->PeekChar(c))
		{
			if (ctype_digit(c))
			{
				token = this->ReadNumber();
			}
			else if (ctype_ident_start(c))
			{
				token = this->ReadIdentifier();
			}
			else if (ctype_symbol(c))
			{
				token = this->ReadSymbol();
			}
			else if (ctype_op(c)) // Assumes '/' is a valid operator
			{
				char cc = '\0';
				token = this->ReadOperator();

				if (int(token.data) == UOP2('/', '\0') && this->PeekChar(cc) && cc == '/') // Line-comments
				{
					while (c != '\n')
						this->GetChar(c);

					return this->ReadToken();
				}
			}
			else if (c == '"')
			{
				token = this->ReadString();
			}

			if (token.type == Token::Invalid)
				throw Lexer_Error("Unknown token", this->row, this->col);

			return token;
		}

		token = Token(Token::EndOfFile);

		return token;
	}
}
