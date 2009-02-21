#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>
#include <cstring>
#include <stdint.h>
#include <cstddef>
#include <cstdarg>
#include <stdexcept>

/**
 * Generic and simple array class.
 */
template <typename T, std::size_t size> class array
{
	private:
		T data[size];

	public:
		array()
		{
			memset((void *)this->data, 0, sizeof(T) * size);
		}

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

		T &operator[](std::size_t index)
		{
			if (index < 0 || index >= size)
			{
				throw std::out_of_range("Out of range accessing array.");
			}

			return this->data[index];
		}
};

/**
 * Commonly used array type in EOSERV.
 */
typedef array<uint8_t, 2> pairchar;

/**
 * Commonly used array type in EOSERV.
 */
typedef array<uint8_t, 4> quadchar;

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

#endif // UTIL_HPP_INCLUDED
