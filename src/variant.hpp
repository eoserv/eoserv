#ifndef VARIANT_HPP_INCLUDED
#define VARIANT_HPP_INCLUDED

#include <string>

class Variant;

class Variant
{
	protected:
		union
		{
			int val_int;
			double val_float;
		};
		std::string val_string;

		enum var_type
		{
			type_int,
			type_float,
			type_string
		};

		int type;

		int GetInt();
		double GetFloat();
		std::string GetString();

		Variant &SetInt(int);
		Variant &SetFloat(double);
		Variant &SetString(const std::string &);

		int int_length(int);

	public:
		Variant();
		Variant(int);
		Variant(float);
		Variant(double);
		Variant(const std::string &);

		Variant &operator =(int);
		Variant &operator =(float);
		Variant &operator =(double);
		Variant &operator =(const std::string &);

		operator int();
		operator float();
		operator double();
		operator std::string();
};

typedef Variant variant;
typedef Variant var;

#endif // VARIANT_HPP_INCLUDED
