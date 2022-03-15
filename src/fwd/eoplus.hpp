/* fwd/eoplus.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_EOPLUS_HPP_INCLUDED
#define FWD_EOPLUS_HPP_INCLUDED

namespace EOPlus
{
	// UOP2 is a hash function which must give a unique value for all operator pairs
	// UOP1 is an alias for single character operators
	// UOP_ALT is an unused symbol that marks operators with 2 meanings
#define UOP2(a, b) ((unsigned char)((((unsigned char)(a) - 32)) + (((unsigned char)(b)) << 2)))
#define UOP1(a) UOP2(a, '\0')
#define UOP_ALT '\2'

	enum OperatorAssoc
	{
		ASSOC_NONE,  // For unary and meta-operators (parentheses)
		ASSOC_RIGHT, // x+y+z is (x+y)+z
		ASSOC_LEFT,  // x=y=z is x=(y=z)
	};

	enum OperatorArgs
	{
		OP_UNARY  = 1, // Operator is prefixed to an argument (e.g. -x, !foo)
		OP_BINARY = 2 // Operator sits between 2 arguments (e.g. x-y, foo=bar)
	};

	enum class Operator : unsigned char
    {
		LeftParens       = UOP1('('),
		RightParens      = UOP1(')'),
		BitAnd           = UOP1('&'),
		LogicalAnd       = UOP2('&', '&'),
		BitOr            = UOP1('|'),
		LogicalOr        = UOP2('|', '|'),
		Assign           = UOP1('='),
		Equality         = UOP2('=', '='),
		Inequality       = UOP2('!', '='),
		LessThan         = UOP1('<'),
		LessThanEqual    = UOP2('<', '='),
		GreaterThan      = UOP1('>'),
		GreaterThanEqual = UOP2('>', '='),
		Add              = UOP1('+'),
		Subtract         = UOP1('-'),
		Multiply         = UOP1('*'),
		Divide           = UOP1('/'),
		Modulo           = UOP1('%'),
		BitNot           = UOP1('~'),
		LogicalNot       = UOP1('!'),
		Negate           = UOP2('-', UOP_ALT)
	};

	struct OperatorInfo;
	struct Token;
	struct Scope;
	struct Expression;
	struct Action;
	struct Rule;
	struct Info;
	struct State;
	struct Quest;

	struct Syntax_Error;
	struct Lexer_Error;
	struct Parser_Error;
	struct Runtime_Error;
}

#endif // FWD_EOPLUS_HPP_INCLUDED
