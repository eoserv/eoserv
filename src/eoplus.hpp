/* eoplus.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOPLUS_HPP_INCLUDED
#define EOPLUS_HPP_INCLUDED

#include "fwd/eoplus.hpp"

#include "util/variant.hpp"

#include <cstddef>
#include <deque>
#include <iosfwd>
#include <map>
#include <stdexcept>
#include <string>

namespace EOPlus
{
	struct OperatorInfo
	{
		Operator op;         // UOP2 hash
		OperatorArgs args;   // Number of arguments (1 or 2) (see op_args)
		char prec;           // Precedence (0-n)
		OperatorAssoc assoc; // Associativity (see op_assoc)
	};

	struct Token
	{
		enum TokenType
		{
			Invalid    = 0,
			Identifier = 1,
			String     = 2,
			Integer    = 4,
			Float      = 8,
			Boolean    = 16,
			Operator   = 32,
			Symbol     = 64,
			NewLine    = 128,
			EndOfFile  = 256
		};

		TokenType type;
		util::variant data;

		int newlines; // Parser uses this to keep track of how many preceeding newline tokens there were

		Token(TokenType type = Invalid, util::variant data = util::variant())
			: type(type)
			, data(data)
			, newlines(0)
		{ }

		explicit operator bool()
		{
			return this->type != Invalid;
		}
	};

	struct Scope
	{
		enum Type
		{
			Default,
			Character,
			NPC,
			Map,
			World
		};

		Type type;
		std::deque<util::variant> args;

		Scope()
			: type(Default)
		{ }
	};

	struct Expression
	{
		std::deque<Scope> scopes;
		std::string function;
		std::deque<util::variant> args;
	};

	struct Action
	{
		enum ConditionalType
		{
			None,
			If,
			Else,
			ElseIf
		};

		ConditionalType cond;
		Expression cond_expr;

		Expression expr;
	};

	struct Rule
	{
		Expression expr;
		Action action;
	};

	struct Info
	{
		enum HiddenType
		{
			NotHidden,
			Hidden,
			HiddenEnd
		};

		std::string name;
		unsigned int version;
		HiddenType hidden;
		bool disabled;

		Info()
			: version(0)
			, hidden(NotHidden)
			, disabled(false)
		{ }
	};

	struct State
	{
		std::string name;
		bool has_desc;
		std::string desc;
		std::deque<Rule> rules;
		std::deque<Action> actions;
		std::size_t goal_rule;

		State()
			: has_desc(false)
			, goal_rule(0)
		{ }
	};

	struct Quest
	{
		bool has_info;
		Info info;
		std::map<std::string, State> states;

		explicit Quest(std::istream &is);
	};

	struct Syntax_Error : public std::runtime_error
	{
		private:
			int line_;

		public:
			Syntax_Error(const std::string &what_, int line_)
				: runtime_error(what_)
				, line_(line_)
			{ }

			int line() const
			{
				return line_;
			}
	};

	struct Lexer_Error : public Syntax_Error
	{
		private:
			int line_;
			int col_;

		public:
			Lexer_Error(const std::string &what_, int line_, int col_)
				: Syntax_Error(what_, line_)
				, col_(col_)
			{ }

			int line() const
			{
				return line_;
			}

			int col() const
			{
				return col_;
			}
	};

	struct Parser_Error : public Syntax_Error
	{
		private:
			int line_;

		public:
			Parser_Error(const std::string &what_, int line_)
				: Syntax_Error(what_, line_)
			{ }

			int line() const
			{
				return line_;
			}
	};

	struct Runtime_Error : public std::runtime_error
	{
		public:
			Runtime_Error(const std::string &what_)
				: runtime_error(what_)
			{ }
	};
}

#endif // EOPLUS_HPP_INCLUDED
