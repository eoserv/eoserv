
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <stack>
#include <unordered_map>
#include <vector>

/**
 * Utility functions to assist with common tasks
 */
namespace util
{

// std::begin/end Not implemented in gcc 4.5

template <class T> inline auto begin(T &c) -> decltype(c.begin()) { return c.begin(); }
template <class T> inline auto begin(const T &c) -> decltype(c.begin()) { return c.begin(); }
template <class T, size_t N> inline const T* begin(T(&a)[N]) { return a; }
template <class T> inline auto end(T &c) -> decltype(c.begin()) { return c.end(); }
template <class T> inline auto end(const T &c) -> decltype(c.begin()) { return c.end(); }
template <class T, size_t N> inline const T* end(T(&a)[N]) { return a + N; }

template <class T> inline auto cbegin(T &c) -> decltype(c.cbegin()) { return c.cbegin(); }
template <class T> inline auto cbegin(const T &c) -> decltype(c.cbegin()) { return c.cbegin(); }
template <class T, size_t N> inline const T* cbegin(T(&a)[N]) { return a; }
template <class T> inline auto cend(T &c) -> decltype(c.cbegin()) { return c.cend(); }
template <class T> inline auto cend(const T &c) -> decltype(c.cbegin()) { return c.cend(); }
template <class T, size_t N> inline const T* cend(T(&a)[N]) { return a + N; }

// C++0x for-range loops are still fairly unsupported

#define UTIL_RANGE(c) (util::begin(c)), (util::end(c))
#define UTIL_CRANGE(c) (util::cbegin(c)), (util::cend(c))

#define UTIL_FOREACH_GENERIC(begin, end, as) \
	if (int _util_continue_ = 1) \
		for (auto _util_it_ = begin; _util_it_ != end && _util_continue_--; ++_util_it_) \
				for (auto as = *_util_it_; !_util_continue_; _util_continue_ = 1)

#define UTIL_FOREACH(c, as) UTIL_FOREACH_GENERIC(util::begin(c), util::end(c), as)
#define UTIL_CFOREACH(c, as) UTIL_FOREACH_GENERIC(util::cbegin(c),util::cend(c), as)
#define UTIL_RANGE_FOREACH(begin, end, as) UTIL_FOREACH_GENERIC(begin, end, as)

#define UTIL_IFOREACH(c, as) for (auto as = util::begin(c); as != util::end(c); ++as)
#define UTIL_CIFOREACH(c, as) for (auto as = util::cbegin(c); as != util::cend(c); ++as)
#define UTIL_RANGE_IFOREACH(begin, end, as) for (auto as = (begin); as != (end); ++as)

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
 * A string wrapper which automatically nulls out it's memory buffer when
 * destroyed or overwritten
 */
struct secure_string
{
	private:
		std::string str_;

	public:
		secure_string(std::string&& s)
			: str_(s)
		{
			std::fill(UTIL_RANGE(s), '\0');
			s.erase();
		}

		secure_string(const secure_string& other)
		{
			erase();
			this->str_ = other.str_;
		}

		secure_string& operator =(const secure_string& rhs)
		{
			erase();
			this->str_ = rhs.str_;
			return *this;
		}

		secure_string& operator =(secure_string&& rhs)
		{
			erase();
			this->str_ = rhs.str_;
			rhs.erase();
			return *this;
		}

		secure_string& operator =(std::string&& rhs)
		{
			erase();
			this->str_ = rhs;
			std::fill(UTIL_RANGE(rhs), '\0');
			rhs.erase();
			return *this;
		}

		void erase()
		{
			std::fill(UTIL_RANGE(this->str_), '\0');
			this->str_.erase();
		}

		/// Make sure nothing copies or mutates this string!
		const std::string& str()
		{
			return this->str_;
		}

		~secure_string()
		{
			erase();
		}
};

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
 * Split a string in to a vector with a specified delimiter
 */
std::vector<std::string> explode(char delimiter, std::string);

/**
 * Split a string in to a vector with a specified delimiter
 */
std::vector<std::string> explode(std::string delimiter, std::string);

/**
 * Alternate name for variant.
 */
typedef variant var;

/**
 * Parse a string time period to a number
 * @param timestr amount of time in a human readable format (eg. 2h30m)
 * @return number of seconds
 */
double tdparse(std::string timestr);

std::stack<util::variant> rpn_parse(std::string expr);
double rpn_eval(std::stack<util::variant>, std::unordered_map<std::string, double> vars);

int to_int(const std::string &);
unsigned int to_uint_raw(const std::string &);
double to_float(const std::string &);

std::string to_string(int);
std::string to_string(double);

std::string lowercase(std::string);

std::string uppercase(std::string);

std::string ucfirst(std::string);

int rand(int min, int max);
double rand(double min, double max);

double round(double);

std::string timeago(double time, double current_time);

void sleep(double seconds);

int text_width(std::string string);
int text_max_word_width(std::string string);
std::string text_cap(std::string string, int width, std::string elipses = "...");
std::string text_word_wrap(std::string string, int width);

/**
 * Finds the distance IN TILES between a pair of x,y coordinates
 */
inline int path_length(int x1, int y1, int x2, int y2)
{
	int dx = std::abs(x1 - x2);
	int dy = std::abs(y1 - y2);

	return dx + dy;
}

}

#endif // UTIL_HPP_INCLUDED
