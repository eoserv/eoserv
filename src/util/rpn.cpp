
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "rpn.hpp"

#include "../util.hpp"
#include "../util/variant.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace util
{

static void rpn_parse_str_reverse(std::string &str)
{
	char temp;
	std::size_t begin = 0;
	std::size_t end = str.length() - 1;

	while (begin < end)
	{
		temp = str[begin];
		str[begin] = str[end];
		str[end] = temp;

		++begin;
		--end;
	}
}

std::stack<util::variant> rpn_parse(std::string expr)
{
	std::stack<util::variant> stack;
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
			stack.push(util::variant(tok));
			tok.clear();
		}
	} while (i-- != 0);

	if (!tok.empty())
	{
		rpn_parse_str_reverse(tok);
		stack.push(util::variant(tok));
	}

	return stack;
}

static double rpn_eval_add(std::vector<double> args)   { return args[0] + args[1]; }
static double rpn_eval_sub(std::vector<double> args)   { return args[0] - args[1]; }
static double rpn_eval_mul(std::vector<double> args)   { return args[0] * args[1]; }
static double rpn_eval_div(std::vector<double> args)   { return args[0] / args[1]; }
static double rpn_eval_mod(std::vector<double> args)   { return int(std::floor(args[0] + 0.5)) % int(std::floor(args[1] + 0.5)); }
static double rpn_eval_and(std::vector<double> args)   { return int(std::floor(args[0] + 0.5)) & int(std::floor(args[1] + 0.5)); }
static double rpn_eval_or(std::vector<double> args)    { return int(std::floor(args[0] + 0.5)) | int(std::floor(args[1] + 0.5)); }
static double rpn_eval_xor(std::vector<double> args)   { return int(std::floor(args[0] + 0.5)) ^ int(std::floor(args[1] + 0.5)); }
static double rpn_eval_not(std::vector<double> args)   { return ~int(std::floor(args[0] + 0.5)); }
static double rpn_eval_pow(std::vector<double> args)   { return std::pow(args[0], args[1]); }
static double rpn_eval_log(std::vector<double> args)   { return std::log10(args[0]); }
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
static double rpn_eval_gte(std::vector<double> args)   { return args[0] >= args[1] - rpn_cmp_epsilon; }
static double rpn_eval_gt(std::vector<double> args)    { return args[0] > args[1] + rpn_cmp_epsilon; }

static double rpn_eval_iif(std::vector<double> args)   { return std::floor(args[0] + 0.5) ? args[1] : args[2]; }

double rpn_eval(std::stack<util::variant> stack, std::unordered_map<std::string, double> vars)
{
	struct rpn_eval_func
	{
		char op;
		const char *name;
		std::size_t args;
		double (*func)(std::vector<double>);

		rpn_eval_func(char op_, const char *name_, std::size_t args_, double (*func_)(std::vector<double>)) : op(op_), name(name_), args(args_), func(func_) { }
	};

	rpn_eval_func rpn_eval_funcs[] = {
		rpn_eval_func('+', "add",   2, rpn_eval_add),
		rpn_eval_func('-', "sub",   2, rpn_eval_sub),
		rpn_eval_func('*', "mul",   2, rpn_eval_mul),
		rpn_eval_func('/', "div",   2, rpn_eval_div),
		rpn_eval_func('%', "mod",   2, rpn_eval_mod),
		rpn_eval_func('&', "and",   2, rpn_eval_and),
		rpn_eval_func('|', "or",    2, rpn_eval_or),
		rpn_eval_func('^', "xor",   2, rpn_eval_xor),
		rpn_eval_func('~', "not",   1, rpn_eval_not),
		rpn_eval_func(' ', "pow",   2, rpn_eval_pow),
		rpn_eval_func(' ', "sqrt",  1, rpn_eval_sqrt),
		rpn_eval_func(' ', "log",   1, rpn_eval_log),
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
		rpn_eval_func(' ', "lte",   2, rpn_eval_lte),
		rpn_eval_func('=', "eq",    2, rpn_eval_eq),
		rpn_eval_func(' ', "gte",   2, rpn_eval_gte),
		rpn_eval_func('>', "gt",    2, rpn_eval_gt),

		rpn_eval_func('?', "iif",   3, rpn_eval_iif),
	};

	std::stack<double> argstack;

	while (!stack.empty())
	{
		start_of_loop:

		util::variant val = stack.top();
		stack.pop();

		for (std::size_t i = 0; i < sizeof(rpn_eval_funcs) / sizeof(rpn_eval_func); ++i)
		{
			rpn_eval_func &func = rpn_eval_funcs[i];

			if (static_cast<std::string>(val) == func.name || (func.op != ' ' && static_cast<std::string>(val)[0] == func.op))
			{
				std::vector<double> args(func.args);

				for (std::size_t i = 0; i < func.args; ++i)
				{
					if (argstack.empty())
					{
						throw std::runtime_error("RPN Stack underflow");
					}

					args[i] = argstack.top();
					argstack.pop();
				}

				argstack.push(func.func(args));

				if (stack.empty()) return argstack.top();

				goto start_of_loop;
			}
		}

		std::unordered_map<std::string, double>::iterator findvar = vars.find(val);

		if (findvar != vars.end())
		{
			argstack.push(findvar->second);
		}
		else
		{
			argstack.push(static_cast<double>(val));
		}
	}

	return argstack.top();
}

}
