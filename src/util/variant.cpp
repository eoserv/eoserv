/* util/variant.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "variant.hpp"

#include "../util.hpp"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>

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
	int count = 0;

	if (x == 0)
		return 1;

	if (x < 0)
	{
		x = -x;
		++count;
	}

	if (x < 0)
	{
		return 1;
	}

	while (x > 0)
	{
		x /= 10;
		++count;
	}

	return count;
}

int variant::GetInt() const
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

double variant::GetFloat() const
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

std::string variant::GetString() const
{
	if (this->cache_val[type_string])
	{
		return this->val_string;
	}
	this->cache_val[type_string] = true;

	char buf[1024];

	using namespace std;

	switch (this->type)
	{
		case type_int:
			snprintf(buf, 1024, "%i", this->val_int);
			this->val_string = buf;
			break;

		case type_float:
			snprintf(buf, 1024, "%g", this->val_float);
			this->val_string = buf;
			break;

		case type_bool:
			this->val_string = this->val_bool ? "yes" : "no";
			break;

		default: ; // Shut the compiler up
	}

	return this->val_string;
}

bool variant::GetBool() const
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

}
