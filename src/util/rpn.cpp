/* util/rpn.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "rpn.hpp"

#include "../util.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "rpn_lex.hpp"

namespace util
{

// Re-purposing unprintable characters that are unlikely to be used in a formula
static const char op_ne = 0x02;
static const char op_lte = 0x03;
static const char op_gte = 0x04;
static const char op_and = 0x05;
static const char op_or = 0x06;
static const char op_neg = 0x07;

static void rpn_map_multichar_op(std::string& str)
{
	if (str == "==")
		str = '=';
	else if (str == "!=")
		str = op_ne;
	else if (str == "<=")
		str = op_lte;
	else if (str == ">=")
		str = op_gte;
	else if (str == "&&")
		str = op_and;
	else if (str == "||")
		str = op_or;
}

static void rpn_parse_str_reverse(std::string &str)
{
	std::reverse(str.begin(), str.end());
}

std::stack<std::string> rpn_parse(std::string expr)
{
	std::stack<std::string> stack;
	std::string tok;

	std::size_t i = expr.length() - 1;
	do
	{
		if (expr[i] != ' ')
		{
			tok += expr[i];
		}
		else if (!tok.empty())
		{
			rpn_parse_str_reverse(tok);
			rpn_map_multichar_op(tok);
			stack.push(tok);
			tok.clear();
		}
	} while (i-- != 0);

	if (!tok.empty())
	{
		rpn_parse_str_reverse(tok);
		rpn_map_multichar_op(tok);
		stack.push(tok);
	}

	return stack;
}

std::stack<std::string> rpn_parse_v2(std::string expr)
{
	static const int prec_value = 0;
	static const int prec_unary = 1;
	static const int prec_comma = 98;
	static const int prec_paren = 99;

	auto prec_is_op = [](int prec) { return prec > 0 && prec < 98; };

	auto op_prec = [](char c)
	{
		switch (c)
		{
			case '!': case '~':
				return 2;
			case '*': case '/': case '%':
				return 3;
			case '+': case '-':
				return 4;
			case '<': case op_lte: case '>': case op_gte:
				return 5;
			case '=': case op_ne:
				return 6;
			case '^':
				return 7;
			case '|':
				return 8;
			case '&':
				return 9;
			case op_and:
				return 10;
			case op_or:
				return 11;
			case ',':
				return prec_comma;
			case '(': case ')':
				return prec_paren;
		}

		return prec_value;
	};

	// The result is built up in reverse order
	std::deque<std::string> stack;
	struct op_stack_t { std::string tok; int prec; };
	std::stack<op_stack_t> op_stack;

	std::stringstream ss(expr);
	std::deque<std::string> tokens;
	Lexer lexer(ss);
	lexer.Lex(std::front_inserter(tokens));

	for (auto it = tokens.begin(); it != tokens.end(); ++it)
	{
		auto& tok = *it;
		rpn_map_multichar_op(tok);
		int prec = op_prec(tok[0]);

		if (prec == prec_value)
		{
			stack.push_front(tok);
			continue;
		}

		if (prec == prec_paren && tok[0] == ')')
		{
			op_stack.push({tok, prec});
			continue;
		}

		if (prec_is_op(prec))
		{
			bool is_unary = false;

			if ((it + 1) == tokens.end())
			{
				is_unary = true;
			}
			else
			{
				auto& prev_tok = *(it + 1);
				int prev_prec = op_prec(prev_tok[0]);

				if (prev_prec != 0 && prev_tok != ")")
					is_unary = true;
			}

			if (is_unary)
			{
				prec = prec_unary;

				if (tok[0] == '-')
					tok = op_neg;

				while (!op_stack.empty() && (op_stack.top().prec <= prec))
				{
					stack.push_front(op_stack.top().tok);
					op_stack.pop();
				}
			}
			else
			{
				while (!op_stack.empty() && (op_stack.top().prec < prec))
				{
					stack.push_front(op_stack.top().tok);
					op_stack.pop();
				}
			}
		}
		else if (prec == prec_comma)
		{
			while (!op_stack.empty() && (op_stack.top().prec <= prec))
			{
				stack.push_front(op_stack.top().tok);
				op_stack.pop();
			}
		}
		else if (prec == prec_paren)
		{
			while (!op_stack.empty())
			{
				if (op_stack.top().prec == prec_paren)
				{
					op_stack.pop();
					break;
				}

				stack.push_front(op_stack.top().tok);
				op_stack.pop();
			}
		}

		if (prec_is_op(prec))
			op_stack.push({tok, prec});
	}

	for (; !op_stack.empty(); op_stack.pop())
	{
		int prec = op_stack.top().prec;
		if (prec_is_op(prec))
			stack.push_front(op_stack.top().tok);
	}

	return std::stack<std::string>(std::move(stack));
}

static int d2i(double d)
{
	return int(std::floor(d + 0.5));
}

static double rpn_eval_add(std::vector<double> args)   { return args[0] + args[1]; }
static double rpn_eval_sub(std::vector<double> args)   { return args[0] - args[1]; }
static double rpn_eval_mul(std::vector<double> args)   { return args[0] * args[1]; }
static double rpn_eval_div(std::vector<double> args)   { return args[0] / args[1]; }
static double rpn_eval_mod(std::vector<double> args)   { return d2i(args[0]) % d2i(args[1]); }
static double rpn_eval_bitand(std::vector<double> args){ return d2i(args[0]) & d2i(args[1]); }
static double rpn_eval_bitor(std::vector<double> args) { return d2i(args[0]) | d2i(args[1]); }
static double rpn_eval_bitxor(std::vector<double> args){ return d2i(args[0]) ^ d2i(args[1]); }
static double rpn_eval_bitnot(std::vector<double> args){ return ~d2i(args[0]); }
static double rpn_eval_and(std::vector<double> args)   { return d2i(args[0]) && d2i(args[1]); }
static double rpn_eval_or(std::vector<double> args)    { return d2i(args[0]) || d2i(args[1]); }
static double rpn_eval_not(std::vector<double> args)   { return !d2i(args[0]); }
static double rpn_eval_neg(std::vector<double> args)   { return -args[0]; }
static double rpn_eval_pow(std::vector<double> args)   { return std::pow(args[0], args[1]); }
static double rpn_eval_log(std::vector<double> args)   { return std::log10(args[0]); }
static double rpn_eval_exp(std::vector<double> args)   { return std::exp(args[0]); }
static double rpn_eval_ln(std::vector<double> args)    { return std::log(args[0]); }
static double rpn_eval_sqrt(std::vector<double> args)  { return std::sqrt(args[0]); }
static double rpn_eval_sin(std::vector<double> args)   { return std::sin(args[0]); }
static double rpn_eval_cos(std::vector<double> args)   { return std::cos(args[0]); }
static double rpn_eval_tan(std::vector<double> args)   { return std::tan(args[0]); }
static double rpn_eval_rand(std::vector<double> args)  { return rand(args[0], args[1]); }
static double rpn_eval_min(std::vector<double> args)   { return std::min(args[0], args[1]); }
static double rpn_eval_max(std::vector<double> args)   { return std::max(args[0], args[1]); }
static double rpn_eval_ceil(std::vector<double> args)  { return std::ceil(args[0]); }
static double rpn_eval_round(std::vector<double> args) { return std::floor(args[0] + 0.5); }
static double rpn_eval_floor(std::vector<double> args) { return std::floor(args[0]); }
static double rpn_eval_lt(std::vector<double> args)    { return args[0] < args[1] - rpn_cmp_epsilon; }
static double rpn_eval_lte(std::vector<double> args)   { return args[0] <= args[1] + rpn_cmp_epsilon; }
static double rpn_eval_eq(std::vector<double> args)    { return args[0] >= args[1] - rpn_cmp_epsilon_2 && args[0] <= args[1] + rpn_cmp_epsilon_2; }
static double rpn_eval_ne(std::vector<double> args)    { return args[0] < args[1] - rpn_cmp_epsilon_2 || args[0] > args[1] + rpn_cmp_epsilon_2; }
static double rpn_eval_gte(std::vector<double> args)   { return args[0] >= args[1] - rpn_cmp_epsilon; }
static double rpn_eval_gt(std::vector<double> args)    { return args[0] > args[1] + rpn_cmp_epsilon; }

static double rpn_eval_iif(std::vector<double> args)   { return d2i(args[0]) ? args[1] : args[2]; }

double rpn_eval(std::stack<std::string> stack, const std::unordered_map<std::string, double>& vars)
{
	struct rpn_eval_func
	{
		char op;
		const char *name;
		std::size_t args;
		double (*func)(std::vector<double>);

		rpn_eval_func(char op_, const char *name_, std::size_t args_, double (*func_)(std::vector<double>)) : op(op_), name(name_), args(args_), func(func_) { }
	};

	static const rpn_eval_func rpn_eval_funcs[] = {
		rpn_eval_func('+', "add",   2, rpn_eval_add),
		rpn_eval_func('-', "sub",   2, rpn_eval_sub),
		rpn_eval_func('*', "mul",   2, rpn_eval_mul),
		rpn_eval_func('/', "div",   2, rpn_eval_div),
		rpn_eval_func('%', "mod",   2, rpn_eval_mod),
		rpn_eval_func('&', "bitand",2, rpn_eval_bitand),
		rpn_eval_func('|', "bitor", 2, rpn_eval_bitor),
		rpn_eval_func('^', "bitxor",2, rpn_eval_bitxor),
		rpn_eval_func('~', "bitnot",1, rpn_eval_bitnot),
		rpn_eval_func(op_and,"and", 2, rpn_eval_and),
		rpn_eval_func(op_or,"or",   2, rpn_eval_or),
		rpn_eval_func('!', "not",   1, rpn_eval_not),
		rpn_eval_func(op_neg,"neg", 1, rpn_eval_neg),
		rpn_eval_func(' ', "pow",   2, rpn_eval_pow),
		rpn_eval_func(' ', "sqrt",  1, rpn_eval_sqrt),
		rpn_eval_func(' ', "log",   1, rpn_eval_log),
		rpn_eval_func(' ', "exp",   1, rpn_eval_exp),
		rpn_eval_func(' ', "ln",    1, rpn_eval_ln),
		rpn_eval_func(' ', "sin",   1, rpn_eval_sin),
		rpn_eval_func(' ', "cos",   1, rpn_eval_cos),
		rpn_eval_func(' ', "tan",   1, rpn_eval_tan),
		rpn_eval_func(' ', "rand",  2, rpn_eval_rand),
		rpn_eval_func(' ', "min",   2, rpn_eval_min),
		rpn_eval_func(' ', "max",   2, rpn_eval_max),
		rpn_eval_func(' ', "ceil",  1, rpn_eval_ceil),
		rpn_eval_func(' ', "round", 1, rpn_eval_round),
		rpn_eval_func(' ', "floor", 1, rpn_eval_floor),
		rpn_eval_func('<', "lt",    2, rpn_eval_lt),
		rpn_eval_func(op_lte,"lte", 2, rpn_eval_lte),
		rpn_eval_func('=', "eq",    2, rpn_eval_eq),
		rpn_eval_func(op_ne,"ne",   2, rpn_eval_ne),
		rpn_eval_func(op_gte,"gte", 2, rpn_eval_gte),
		rpn_eval_func('>', "gt",    2, rpn_eval_gt),

		rpn_eval_func('?', "iif",   3, rpn_eval_iif),
	};

	std::stack<double> argstack;

	while (!stack.empty())
	{
		start_of_loop:

		const std::string& val = stack.top();
		stack.pop();

		for (std::size_t i = 0; i < sizeof(rpn_eval_funcs) / sizeof(rpn_eval_func); ++i)
		{
			const rpn_eval_func &func = rpn_eval_funcs[i];

			if (val == func.name || (func.op != ' ' && val[0] == func.op))
			{
				std::vector<double> args(func.args);

				for (std::size_t ii = 0; ii < func.args; ++ii)
				{
					if (argstack.empty())
					{
						throw std::runtime_error("RPN Stack underflow");
					}

					args[ii] = argstack.top();
					argstack.pop();
				}

				argstack.push(func.func(args));

				if (stack.empty())
					return argstack.top();

				goto start_of_loop;
			}
		}

		auto findvar = vars.find(val);

		if (findvar != vars.end())
		{
			argstack.push(findvar->second);
		}
		else
		{
			argstack.push(util::tdparse(val));
		}
	}

	if (argstack.empty())
		return 0.0;

	return argstack.top();
}

}
