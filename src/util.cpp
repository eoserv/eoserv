
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "util.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <limits>
#include <stdexcept>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif // defined(WIN32) || defined(WIN64)

namespace util
{

variant::variant()
{
	this->SetInt(0);
}

variant::variant(int i)
{
	this->SetInt(i);
}

variant::variant(double d)
{
	this->SetFloat(d);
}

variant::variant(const std::string &s)
{
	this->SetString(s);
}

variant::variant(const char *p)
{
	std::string s(p);
	this->SetString(s);
}

variant::variant(bool b)
{
	this->SetBool(b);
}

int variant::int_length(int x)
{
	int count = 1;
	int val = 10;

	while (x >= val)
	{
		val *= 10;
		++count;
	}

	return count;
}

int variant::GetInt()
{
	if (this->cache_val[type_int])
	{
		return this->val_int;
	}
	this->cache_val[type_int] = true;

	switch (this->type)
	{
		case type_float:
			this->val_int = static_cast<int>(this->val_float);
			break;

		case type_string:
			this->val_int = tdparse(this->val_string);
			break;

		case type_bool:
			this->val_int = this->val_bool ? 0 : -1;
			break;

		default: ; // Shut the compiler up
	}

	return this->val_int;
}

double variant::GetFloat()
{
	if (this->cache_val[type_float])
	{
		return this->val_float;
	}
	this->cache_val[type_float] = true;

	switch (this->type)
	{
		case type_int:
			this->val_float = static_cast<double>(this->val_int);
			break;

		case type_string:
			this->val_float = tdparse(this->val_string);
			break;

		case type_bool:
			this->val_float = this->val_bool ? 0.0 : 1.0;
			break;

		default: ; // Shut the compiler up
	}

	return this->val_float;
}

std::string variant::GetString()
{
	if (this->cache_val[type_string])
	{
		return this->val_string;
	}
	this->cache_val[type_string] = true;

	char buf[1024];

	switch (this->type)
	{
		case type_int:
			snprintf(buf, 1024, "%i", this->val_int);
			this->val_string = buf;
			break;

		case type_float:
			snprintf(buf, 1024, "%lf", this->val_float);
			this->val_string = buf;
			break;

		case type_bool:
			this->val_string = this->val_bool ? "yes" : "no";
			break;

		default: ; // Shut the compiler up
	}

	return this->val_string;
}

bool variant::GetBool()
{
	if (this->cache_val[type_bool])
	{
		return this->val_bool;
	}
	this->cache_val[type_bool] = true;

	int intval = 0;
	std::string s = this->val_string;

	switch (this->type)
	{
		case type_int:
			this->val_bool = static_cast<bool>(this->val_int);
			break;

		case type_float:
			this->val_bool = std::abs(this->val_float) != 0.0 && this->val_float == this->val_float;
			break;

		case type_string:
			std::sscanf(this->val_string.c_str(), "%d", &intval);
			s = util::lowercase(s);
			this->val_bool = (s == "yes" || s == "true" || s == "enabled" || intval != 0);
			break;

		default: ; // Shut the compiler up
	}

	return this->val_bool;
}

void variant::SetType(variant::var_type type)
{
	this->type = type;

	for (std::size_t i = 0; i < sizeof(this->cache_val); ++i)
	{
		this->cache_val[i] = (static_cast<variant::var_type>(i) == type);
	}
}

variant &variant::SetInt(int i)
{
	this->val_int = i;
	this->SetType(type_int);
	return *this;
}

variant &variant::SetFloat(double d)
{
	this->val_float = d;
	this->SetType(type_float);
	return *this;
}

variant &variant::SetString(const std::string &s)
{
	this->val_string = s;
	this->SetType(type_string);
	return *this;
}

variant &variant::SetBool(bool b)
{
	this->val_bool = b;
	this->SetType(type_bool);
	return *this;
}

variant &variant::operator =(int i)
{
	return this->SetInt(i);
}

variant &variant::operator =(double d)
{
	return this->SetFloat(d);
}

variant &variant::operator =(const std::string &s)
{
	return this->SetString(s);
}

variant &variant::operator =(const char *p)
{
	std::string s(p);
	return this->SetString(s);
}

variant &variant::operator =(bool b)
{
	return this->SetBool(b);
}

variant::operator int()
{
	return this->GetInt();
}

variant::operator double()
{
	return this->GetFloat();
}

variant::operator std::string()
{
	return this->GetString();
}

variant::operator bool()
{
	return this->GetBool();
}

std::string ltrim(const std::string &str)
{
	std::size_t si = str.find_first_not_of(" \t\n\r");

	if (si == std::string::npos)
	{
		si = 0;
	}
	else
	{
		--si;
	}
	++si;

	return str.substr(si);
}

std::string rtrim(const std::string &str)
{
	std::size_t ei = str.find_last_not_of(" \t\n\r");

	if (ei == std::string::npos)
	{
		ei = str.length()-1;
	}
	++ei;

	return str.substr(0, ei);
}

std::string trim(const std::string &str)
{
	std::size_t si, ei;
	bool notfound = false;

	si = str.find_first_not_of(" \t\n\r");
	if (si == std::string::npos)
	{
		si = 0;
		notfound = true;
	}

	ei = str.find_last_not_of(" \t\n\r");
	if (ei == std::string::npos)
	{
		if (notfound)
		{
			return "";
		}
		ei = str.length()-1;
	}
	++ei;

	return str.substr(si, ei);
}

std::vector<std::string> explode(char delimiter, std::string str)
{
	std::size_t lastpos = 0;
	std::size_t pos = 0;
	std::vector<std::string> pieces;

	for (pos = str.find_first_of(delimiter); pos != std::string::npos; )
	{
		pieces.push_back(str.substr(lastpos, pos - lastpos));
		lastpos = pos+1;
		pos = str.find_first_of(delimiter, pos+1);
	}
	pieces.push_back(str.substr(lastpos));

	return pieces;
}

std::vector<std::string> explode(std::string delimiter, std::string str)
{
	std::size_t lastpos = 0;
	std::size_t pos = 0;
	std::vector<std::string> pieces;

	if (delimiter.length() == 0)
	{
		return pieces;
	}

	if (delimiter.length() == 1)
	{
		return explode(delimiter[0], str);
	}

	for (pos = str.find(delimiter); pos != std::string::npos; )
	{
		pieces.push_back(str.substr(lastpos, pos - lastpos));
		lastpos = pos + delimiter.length();
		pos = str.find(delimiter, pos + delimiter.length());
	}
	pieces.push_back(str.substr(lastpos));

	return pieces;
}

double tdparse(std::string timestr)
{
	static char period_names[] = {'s', 'm',  '%',   'k',    'h',    'd'    };
	static double period_mul[] = {1.0, 60.0, 100.0, 1000.0, 3600.0, 86400.0};
	double ret = 0.0;
	double val = 0.0;
	bool decimal = false;
	double decimalmulti = 0.1;
	bool negate = false;

	for (std::size_t i = 0; i < timestr.length(); ++i)
	{
		char c = timestr[i];
		bool found = false;

		if (c == '-')
		{
			negate = true;
			continue;
		}

		if (c >= 'A' && c <= 'Z')
		{
			c -= 'A' - 'a';
		}

		for (std::size_t ii = 0; ii < sizeof(period_names)/sizeof(char); ++ii)
		{
			if (c == period_names[ii])
			{
				if (c == 'm' && (i < timestr.length()-1 && timestr[i+1] == 's'))
				{
					ret += val / 1000.0;
					++i;
				}
				else if (c == '%')
				{
					ret += val / period_mul[ii];
				}
				else
				{
					ret += val * period_mul[ii];
				}

				found = true;
				val = 0.0;

				decimal = false;
				decimalmulti = 0.1;

				break;
			}
		}

		if (!found)
		{
			if (c >= '0' && c <= '9')
			{
				if (!decimal)
				{
					val *= 10.0;
					val += c - '0';
				}
				else
				{
					val += (c - '0') * decimalmulti;
					decimalmulti /= 10.0;
				}
			}
			else if (c == '.')
			{
				decimal = true;
				decimalmulti = 0.1;
			}
		}
	}

	return (ret + val) * (negate ? -1.0 : 1.0);
}

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
static double rpn_eval_mod(std::vector<double> args)   { return int(args[0]) % int(args[1]); }
static double rpn_eval_and(std::vector<double> args)   { return int(args[0]) & int(args[1]); }
static double rpn_eval_or(std::vector<double> args)    { return int(args[0]) | int(args[1]); }
static double rpn_eval_xor(std::vector<double> args)   { return int(args[0]) ^ int(args[1]); }
static double rpn_eval_not(std::vector<double> args)   { return ~int(args[0]); }
static double rpn_eval_pow(std::vector<double> args)   { return std::pow(args[0], args[1]); }
static double rpn_eval_log(std::vector<double> args)   { return std::log10(args[0]); }
static double rpn_eval_sqrt(std::vector<double> args)  { return std::sqrt(args[0]); }
static double rpn_eval_sin(std::vector<double> args)   { return std::sin(args[0]); }
static double rpn_eval_cos(std::vector<double> args)   { return std::cos(args[0]); }
static double rpn_eval_tan(std::vector<double> args)   { return std::tan(args[0]); }
static double rpn_eval_rand(std::vector<double> args)  { return rand(args[0], args[1]); }
static double rpn_eval_iif(std::vector<double> args)   { return args[0] ? args[1] : args[2]; }
static double rpn_eval_min(std::vector<double> args)   { return std::min(args[0], args[1]); }
static double rpn_eval_max(std::vector<double> args)   { return std::max(args[0], args[1]); }
static double rpn_eval_ceil(std::vector<double> args)  { return std::ceil(args[0]); }
static double rpn_eval_round(std::vector<double> args) { return std::floor(args[0] + 0.5); }
static double rpn_eval_floor(std::vector<double> args) { return std::floor(args[0]); }
static double rpn_eval_lte(std::vector<double> args)   { return args[0] <= args[1]; }
static double rpn_eval_lt(std::vector<double> args)    { return args[0] < args[1]; }
static double rpn_eval_eq(std::vector<double> args)    { return args[0] == args[1]; }
static double rpn_eval_gt(std::vector<double> args)    { return args[0] > args[1]; }
static double rpn_eval_gte(std::vector<double> args)   { return args[0] >= args[1]; }

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
		rpn_eval_func('?', "iif",   3, rpn_eval_iif),
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

int to_int(const std::string &subject)
{
	return static_cast<int>(util::variant(subject));
}

unsigned int to_uint_raw(const std::string &subject)
{
	unsigned int multiplier = 1;
	unsigned int result = 0;

	std::size_t i = subject.length();
	do {
		--i;

		if (subject[i] < '0' || subject[i] > '9')
		{
			throw std::invalid_argument("Non-numeric argument");
		}

		result += (subject[i] - '0') * multiplier;
		multiplier *= 10;
	} while (i > 0);

	return result;
}

double to_float(const std::string &subject)
{
	return static_cast<double>(util::variant(subject));
}

std::string to_string(int subject)
{
	return static_cast<std::string>(util::variant(subject));
}

std::string to_string(double subject)
{
	return static_cast<std::string>(util::variant(subject));
}

std::string lowercase(std::string subject)
{
	std::transform(subject.begin(), subject.end(), subject.begin(), static_cast<int(*)(int)>(std::tolower));
	return subject;
}

std::string uppercase(std::string subject)
{
	std::transform(subject.begin(), subject.end(), subject.begin(), static_cast<int(*)(int)>(std::toupper));
	return subject;
}

std::string ucfirst(std::string subject)
{
	if (subject[0] > 'a' && subject[0] < 'z')
	{
		subject[0] += 'A' - 'a';
	}

	return subject;
}

static void rand_init()
{
	static bool init = false;

	if (!init)
	{
		init = true;
		std::srand(std::time(0));
	}
}

static unsigned long long_rand()
{
#if RAND_MAX < 65535
	return (std::rand() & 0xFF) << 24 | (std::rand() & 0xFF) << 16 | (std::rand() & 0xFF) << 8 | (std::rand() & 0xFF);
#else
#if RAND_MAX < 4294967295
	return (std::rand() & 0xFFFF) << 16 | (std::rand() & 0xFFFF);
#else
	return std::rand();
#endif
#endif
}

int rand(int min, int max)
{
	rand_init();
	rand(double(min), double(max));
	return static_cast<int>(double(long_rand()) / (double(std::numeric_limits<unsigned long>::max()) + 1.0) * double(max - min + 1) + double(min));
}

double rand(double min, double max)
{
	rand_init();
	return double(long_rand()) / double(std::numeric_limits<unsigned long>::max()) * (max - min) + min;
}

double round(double subject)
{
	return std::floor(subject + 0.5);
}

std::string timeago(double time, double current_time)
{
	static bool init = false;
	static std::vector<std::pair<int, std::string>> times;

	if (!init)
	{
		init = true;
		times.resize(5);
		times.push_back(std::make_pair(1, "second"));
		times.push_back(std::make_pair(60, "minute"));
		times.push_back(std::make_pair(60*60, "hour"));
		times.push_back(std::make_pair(24*60*60, "day"));
		times.push_back(std::make_pair(7*24*60*60, "week"));
	}

	std::string ago;

	double diff = current_time - time;

	ago = ((diff >= 0) ? " ago" : " from now");

	diff = std::abs(diff);

	for (int i = times.size()-1; i >= 0; --i)
	{
		int x = int(diff / times[i].first);
		diff -= x * times[i].first;

		if (x > 0)
		{
			return util::to_string(x) + " " + times[i].second + ((x == 1) ? "" : "s") + ago;
		}
	}

	return "now";
}

void sleep(double seconds)
{
#if defined(WIN32) || defined(WIN64)
	Sleep(int(seconds * 1000.0));
#else // defined(WIN32) || defined(WIN64)
	unsigned long sec = seconds;
	unsigned long nsec = (seconds - double(sec)) * 1000000000.0;
	timespec ts = {sec, nsec};
	nanosleep(&ts, 0);
#endif // defined(WIN32) || defined(WIN64)
}

static int sizes[256] = {
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, /* NUL -  SI */
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, /* DLE -  US */
	 3,  3,  5,  7,  6,  8,  6,  2,  3,  3,  4,  6,  3,  3,  3,  5, /* ' ' - '/' */
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  3,  3,  6,  6,  6,  6, /* '0' - '?' */
	11,  7,  7,  7,  8,  7,  6,  8,  8,  3,  5,  7,  6,  9,  8,  8, /* '@' - 'O' */
	 7,  8,  8,  7,  7,  8,  7, 11,  7,  7,  7,  3,  5,  3,  6,  6, /* 'P' - '_' */
	 3,  6,  6,  6,  6,  6,  3,  6,  6,  2,  2,  6,  2,  8,  6,  6, /* '`' - 'o' */
	 6,  6,  3,  5,  3,  6,  6,  8,  5,  5,  5,  4,  2,  4,  7,  3, /* 'p' - DEL */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 3,  3,  6,  6,  6,  6,  2,  6,  3,  9,  4,  6,  6,  3,  8,  6,
	 4,  6,  3,  3,  3,  6,  6,  3,  3,  3,  4,  6,  8,  8,  8,  6,
	 7,  7,  7,  7,  7,  7, 10,  7,  7,  7,  7,  7,  3,  3,  3,  3,
	 8,  8,  8,  8,  8,  8,  8,  6,  8,  8,  8,  8,  8,  7,  7,  6,
	 6,  6,  6,  6,  6,  6, 10,  6,  6,  6,  6,  6,  2,  4,  4,  4,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  5,
};

int text_width(std::string string)
{
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		length += sizes[string[i] & 0xFF];
	}

	return length;
}

int text_max_word_width(std::string string)
{
	int max_length = 0;
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		if (string[i] == ' ')
		{
			length = 0;
			max_length = std::max(max_length, length);
		}
		else
		{
			length += sizes[string[i] & 0xFF];
		}
	}

	return std::max(max_length, length);
}

std::string text_cap(std::string string, int width, std::string elipses)
{
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		length += sizes[string[i] & 0xFF];

		if (length > width)
		{
			int elipses_length = text_width(elipses);

			while (length > (width + elipses_length) && i > 0)
			{
				length -= string[i];
				--i;
			}

			return string.substr(0, i);
		}
	}

	return string;
}

std::string text_word_wrap(std::string string, int width)
{
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		if (string[i] == ' ')
		{
			length = 0;
		}
		else
		{
			length += sizes[string[i] & 0xFF];

			if (length > width)
			{
				string.insert(i - 1, " ");
				length = 0;
			}
		}
	}

	return string;
}

}
