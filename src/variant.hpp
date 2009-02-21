#ifndef VARIANT_HPP_INCLUDED
#define VARIANT_HPP_INCLUDED

#include <string>

class Variant;

/**
 * A type that can store any numeric/string value and convert between them.
 * It takes way too much effort to use, so it's only used by the Config class.
 */
class Variant
{
	protected:
		/**
		 * Integer and float values are a union to save on a few bytes of memory.
		 */
		union
		{
			/**
			 * Value stored as an integer.
			 */
			int val_int;
			
			/**
			 * Value stored as a float.
			 */
			double val_float;
		};
		
		/**
		 * Value stored as a string.
		 */
		std::string val_string;

		enum var_type
		{
			type_int,
			type_float,
			type_string
		};

		/**
		 * Current type the value is stored as.
		 * Accessing as this type will need no conversion.
		 */
		int type;

		/**
		 * Return the value as an integer, casting if neccessary.
		 */
		int GetInt();

		/**
		 * Return the value as a float, casting if neccessary.
		 */
		double GetFloat();

		/**
		 * Return the value as a string, casting if neccessary.
		 */
		std::string GetString();

		/**
		 * Set the value to an integer.
		 */
		Variant &SetInt(int);

		/**
		 * Set the value to a float.
		 */
		Variant &SetFloat(double);

		/**
		 * Set the value to a string.
		 */
		Variant &SetString(const std::string &);

		/**
		 * Helper function that returns the string length of a number in decimal format.
		 */
		int int_length(int);

	public:
		/**
		 * Initialize the variant to an integer with the value 0.
		 */
		Variant();

		/**
		 * Initialize the variant to an integer with the specified value.
		 */
		Variant(int);

		/**
		 * Initialize the variant to a float with the specified value.
		 */
		Variant(double);

		/**
		 * Initialize the variant to a string with the specified value.
		 */
		Variant(const std::string &);

		/**
		 * Set the value to an integer.
		 */
		Variant &operator =(int);

		/**
		 * Set the value to a float.
		 */
		Variant &operator =(double);

		/**
		 * Set the value to a string.
		 */
		Variant &operator =(const std::string &);

		/**
		 * Return the value as an integer, casting if neccessary.
		 */
		operator int();

		/**
		 * Return the value as a float, casting if neccessary.
		 */
		operator double();

		/**
		 * Return the value as a string, casting if neccessary.
		 */
		operator std::string();
};

typedef Variant variant;
typedef Variant var;

#endif // VARIANT_HPP_INCLUDED
