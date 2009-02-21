
#include "variant.hpp"

#include <string>

Variant::Variant()
{
	this->type = type_int;
	this->val_int = 0;
}

Variant::Variant(int i)
{
	this->SetInt(i);
}

Variant::Variant(double d)
{
	this->SetFloat(d);
}

Variant::Variant(const std::string &s)
{
	this->SetString(s);
}

int Variant::int_length(int x)
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

int Variant::GetInt()
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

double Variant::GetFloat()
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

std::string Variant::GetString()
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

Variant &Variant::SetInt(int i)
{
	this->val_int = i;
	this->type = type_int;
	return *this;
}

Variant &Variant::SetFloat(double d)
{
	this->val_float = d;
	this->type = type_float;
	return *this;
}

Variant &Variant::SetString(const std::string &s)
{
	this->val_string = s;
	this->type = type_string;
	return *this;
}
Variant &Variant::operator =(int i)
{
	return this->SetInt(i);
}

Variant &Variant::operator =(double d)
{
	return this->SetFloat(d);
}

Variant &Variant::operator =(const std::string &s)
{
	return this->SetString(s);
}

Variant::operator int()
{
	return this->GetInt();
}

Variant::operator double()
{
	return this->GetFloat();
}

Variant::operator std::string()
{
	return this->GetString();
}

