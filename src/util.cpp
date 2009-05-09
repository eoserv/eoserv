
#include "util.hpp"

#include <list>
#include <vector>
#include <cstdio>
#include <cmath>
#include <ctime>
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
			this->val_int = 0;
			std::sscanf(this->val_string.c_str(), "%d", &this->val_int);
			break;
	}

	return this->val_int;
}

double variant::GetFloat()
{
	switch (this->type)
	{
		case type_int:
			this->val_float = static_cast<double>(this->val_int);
			break;

		case type_string:
			this->val_float = 0.0;
			std::sscanf(this->val_string.c_str(), "%lf", &this->val_float);
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
			std::snprintf(buf, 1024, "%i", this->val_int);
			this->val_string = buf;
			break;

		case type_float:
			std::snprintf(buf, 1024, "%f", this->val_float);
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
	static char period_names[] = {'s', 'm',  'h',    'd'    };
	static double period_mul[] = {1.0, 60.0, 3600.0, 86400.0};
	double ret = 0.0;
	double val = 0;

	for (std::size_t i = 0; i < timestr.length(); ++i)
	{
		char c = timestr.c_str()[i];
		bool found = false;

		if (c >= 'A' && c <= 'Z')
		{
			c -= 'A' - 'a';
		}

		for (std::size_t i = 0; i < sizeof(period_names)/sizeof(char); ++i)
		{
			if (c == period_names[i])
			{
				ret += val * period_mul[i];
				found = true;
				val = false;
			}
		}

		if (!found)
		{
			val *= 10;
			val += c - '0';
		}
	}

	return ret;
}

int to_int(const std::string &subject)
{
	return static_cast<int>(util::variant(subject));
}

std::string to_string(int subject)
{
	return static_cast<std::string>(util::variant(subject));
}

void lowercase(std::string &subject)
{
	std::transform(subject.begin(), subject.end(), subject.begin(), static_cast<int(*)(int)>(std::tolower));
}

void uppercase(std::string &subject)
{
	std::transform(subject.begin(), subject.end(), subject.begin(), static_cast<int(*)(int)>(std::toupper));
}

void ucfirst(std::string &subject)
{
	if (subject[0] > 'a' && subject[0] < 'z')
	{
		subject[0] += 'A' - 'a';
	}
}

int rand(int min, int max)
{
	static bool init = false;
	if (!init)
	{
		init = true;
		std::srand(std::time(0));
	}
	return static_cast<int>(double(std::rand()) / RAND_MAX * (max - min + 1) + min);
}

double round(double subject)
{
	return std::floor(subject + 0.5);
}

}
