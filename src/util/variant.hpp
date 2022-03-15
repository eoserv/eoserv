/* util/variant.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_VARIANT_HPP_INCLUDED
#define UTIL_VARIANT_HPP_INCLUDED

#include <string>

namespace util
{

/**
 * A type that can store any numeric/string value and convert between them.
 * It takes way too much effort to use, so it's only used by the Config class.
 */
class variant
{
	protected:
		/**
		 * Value stored as an integer.
		 */
		mutable int val_int;

		/**
		 * Value stored as a float.
		 */
		mutable double val_float;

		/**
		 * Value stored as a string.
		 */
		mutable std::string val_string;

		/**
		 * Value stored as a bool.
		 */
		mutable bool val_bool;

		enum var_type
		{
			type_int,
			type_float,
			type_string,
			type_bool
		};

		mutable bool cache_val[4];

		/**
		 * Current type the value is stored as.
		 * Accessing as this type will need no conversion.
		 */
		var_type type;

		/**
		 * Invalidates the cache values and changes the type.
		 */
		void SetType(var_type);

		/**
		 * Helper function that returns the string length of a number in decimal format.
		 */
		static int int_length(int);

	public:
		/**
		 * Return the value as an integer, casting if neccessary.
		 */
		int GetInt() const;

		/**
		 * Return the value as a float, casting if neccessary.
		 */
		double GetFloat() const;

		/**
		 * Return the value as a string, casting if neccessary.
		 */
		std::string GetString() const;

		/**
		 * Return the value as a bool, casting if neccessary.
		 */
		bool GetBool() const;

		/**
		 * Set the value to an integer.
		 */
		variant &SetInt(int);

		/**
		 * Set the value to a float.
		 */
		variant &SetFloat(double);

		/**
		 * Set the value to a string.
		 */
		variant &SetString(const std::string &);

		/**
		 * Set the value to a bool.
		 */
		variant &SetBool(bool);

		/**
		 * Initialize the variant to an integer with the value 0.
		 */
		variant();

		/**
		 * Initialize the variant to an integer with the specified value.
		 */
		variant(int);

		/**
		 * Initialize the variant to a float with the specified value.
		 */
		variant(double);

		/**
		 * Initialize the variant to a string with the specified value.
		 */
		variant(const std::string &);

		/**
		 * Initialize the variant to a string with the specified value.
		 */
		variant(const char *);

		/**
		 * Initialize the variant to a bool with the specified value.
		 */
		variant(bool);

		/**
		 * Set the value to an integer.
		 */
		variant &operator =(int);

		/**
		 * Set the value to a float.
		 */
		variant &operator =(double);

		/**
		 * Set the value to a string.
		 */
		variant &operator =(const std::string &);

		/**
		 * Set the value to a string.
		 */
		variant &operator =(const char *);

		/**
		 * Set the value to a bool.
		 */
		variant &operator =(bool);

		/**
		 * Return the value as an integer, casting if neccessary.
		 */
		operator int() const { return this->GetInt(); }

		/**
		 * Return the value as a float, casting if neccessary.
		 */
		operator double() const { return this->GetFloat(); }

		/**
		 * Return the value as a string, casting if neccessary.
		 */
		operator std::string() const { return this->GetString(); }

		/**
		 * Return the value as an boolean, casting if neccessary.
		 */
		operator bool() const { return this->GetBool(); }
};

/**
 * Alternate name for variant.
 */
typedef variant var;

}

#endif // UTIL_VARIANT_HPP_INCLUDED
