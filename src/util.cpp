
#include "util.hpp"

#include <list>
#include <cstdio>
#include <string>

namespace util
{

variant::variant()
{
	this->type = type_int;
	this->val_int = 0;
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
	switch (this->type)
	{
		case type_float:
			this->val_int = static_cast<int>(this->val_float);
			break;

		case type_string:
			sscanf(this->val_string.c_str(), "%d", &this->val_int);
			break;
	}

	return this->val_int;
}

double variant::GetFloat()
{
	switch (this->type)
	{
		case type_int:
			this->val_float = double(this->val_int);
			break;

		case type_string:
			sscanf(this->val_string.c_str(), "%lf", &this->val_float);
			break;
	}

	return this->val_float;
}

std::string variant::GetString()
{
	char buf[1024];

	switch (this->type)
	{
		case type_int:
			snprintf(buf, 1024, "%i", this->val_int);
			this->val_string = buf;
			break;

		case type_float:
			snprintf(buf, 1024, "%f", this->val_float);
			this->val_string = buf;
			break;
	}

	return this->val_string;
}

variant &variant::SetInt(int i)
{
	this->val_int = i;
	this->type = type_int;
	return *this;
}

variant &variant::SetFloat(double d)
{
	this->val_float = d;
	this->type = type_float;
	return *this;
}

variant &variant::SetString(const std::string &s)
{
	this->val_string = s;
	this->type = type_string;
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

std::list<std::string> explode(char delimiter, std::string str)
{
	std::size_t lastpos = 0;
	std::size_t pos = 0;
	std::list<std::string> pieces;

	for (pos = str.find_first_of(delimiter); pos != std::string::npos; )
	{
		pieces.push_back(str.substr(lastpos, pos - lastpos));
		lastpos = pos+1;
		pos = str.find_first_of(delimiter, pos+1);
	}
	pieces.push_back(str.substr(lastpos));

	return pieces;
}

}
