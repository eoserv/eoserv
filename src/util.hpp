#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>
#include <map>
#include <cstring>
#include <cstddef>
#include <cstdarg>
#include <stdexcept>

namespace util
{

// A large pile of hack and fail
#define UTIL_FOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (typeof(*(container.begin())) as; util_i < 1; ++util_i) for (typeof(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = *(util_it)):(as = *container.begin())), util_it != container.end(); as = *(util_it++))

template <typename T, std::size_t size> class array;
class variant;

/**
 * Generic and simple array class.
 */
template <typename T, std::size_t size> class array
{
	private:
		T data[size];

	public:
		array() {}
		array(T init)
		{
			for (std::size_t i = 0; i < size; ++i)
			{
				data[i] = init;
			}
		}

		array(T init[size])
		{
			std::memcpy((void *)this->data, (void *)init, sizeof(T) * size);
		}

		T& assign(std::size_t index, T value)
		{
			if (index >= size)
			{
				throw std::out_of_range("Out of range accessing array.");
			}
			this->data[index] = value;
			return this->data[index];
		}

		T &operator[](std::size_t index)
		{
			if (index >= size)
			{
				throw std::out_of_range("Out of range accessing array.");
			}

			return this->data[index];
		}
};

/**
 * A type that can store any numeric/string value and convert between them.
 * It takes way too much effort to use, so it's only used by the Config class.
 */
class variant
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
		 * Helper function that returns the string length of a number in decimal format.
		 */
		int int_length(int);

	public:
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

/**
 * Commonly used array type in EOSERV.
 */
typedef array<unsigned char, 2> pairchar;

/**
 * Commonly used array type in EOSERV.
 */
typedef array<unsigned char, 4> quadchar;

/**
 * Trims whitespace from the left of a string.
 * Whitespace is defined as space, tab, CR and LF.
 */
std::string ltrim(const std::string &);

/**
 * Trims whitespace from the right of a string.
 * Whitespace is defined as space, tab, CR and LF.
 */
std::string rtrim(const std::string &);

/**
 * Trims whitespace from both sides of a string.
 * Whitespace is defined as space, tab, CR and LF.
 */
std::string trim(const std::string &);

/**
 * Alternate name for variant.
 */
typedef variant var;

}

#endif // UTIL_HPP_INCLUDED
