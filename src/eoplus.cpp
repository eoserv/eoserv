/* eoplus.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoplus.hpp"

#include "eoplus/lex.hpp"
#include "eoplus/parse.hpp"

#include "util.hpp"

#include <cstddef>
#include <deque>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace EOPlus
{
	const OperatorInfo op_info_table[19] = {
		{Operator::LeftParens,       OP_BINARY,  0, ASSOC_NONE},
		{Operator::RightParens,      OP_BINARY,  0, ASSOC_NONE},
		{Operator::BitAnd,           OP_BINARY,  2, ASSOC_RIGHT},
		{Operator::LogicalAnd,       OP_BINARY,  2, ASSOC_RIGHT},
		{Operator::BitOr,            OP_BINARY,  2, ASSOC_RIGHT},
		{Operator::LogicalOr,        OP_BINARY,  2, ASSOC_RIGHT},
		{Operator::Equality,         OP_BINARY,  3, ASSOC_RIGHT},
		{Operator::Inequality,       OP_BINARY,  3, ASSOC_RIGHT},
		{Operator::LessThan,         OP_BINARY,  3, ASSOC_RIGHT},
		{Operator::LessThanEqual,    OP_BINARY,  3, ASSOC_RIGHT},
		{Operator::GreaterThan,      OP_BINARY,  3, ASSOC_RIGHT},
		{Operator::GreaterThanEqual, OP_BINARY,  3, ASSOC_RIGHT},
		{Operator::Add,              OP_BINARY,  5, ASSOC_RIGHT},
		{Operator::Subtract,         OP_BINARY,  5, ASSOC_RIGHT},
		{Operator::Multiply,         OP_BINARY,  6, ASSOC_RIGHT},
		{Operator::Divide,           OP_BINARY,  6, ASSOC_RIGHT},
		{Operator::Modulo,           OP_BINARY,  6, ASSOC_RIGHT},
		{Operator::LogicalNot,       OP_UNARY,   8, ASSOC_NONE},
		{Operator::Negate,           OP_UNARY,   8, ASSOC_NONE}
	};

	OperatorInfo op_info_lut[0x100] = {};

	OperatorInfo op_info(Operator op)
	{
		typedef unsigned char uc;
		return op_info_lut[uc(op)];
	}

	struct op_info_lut_init
	{
		op_info_lut_init()
		{
			typedef unsigned char uc;
			static bool initialized;

			if (!initialized)
			{
				initialized = true;

				for (std::size_t i = 0; i < sizeof(op_info_table) / sizeof(op_info_table[0]); ++i)
				{
					if (op_info_lut[uc(op_info_table[i].op)].op != Operator(0))
						throw std::logic_error("UOP2 hash collision");

					op_info_lut[uc(op_info_table[i].op)] = op_info_table[i];
				}
			}
		}
	};

	op_info_lut_init op_info_lut_init_instance;

	Quest::Quest(std::istream &is)
		: has_info(false)
	{
		std::deque<Token> tokens;

		Lexer lexer(is);
		lexer.Lex(std::back_inserter(tokens));

		Parser parser(*this, UTIL_RANGE(tokens));
		parser.ParseGlobal();
	}
}
