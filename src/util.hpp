
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

template <class IT, class ITE> struct foreach_helper
{
	IT it;
	ITE end;
	int cont;

	bool clause() { return cont-- && it != end; }
	void act()    { if (cont) ++it; }
};

#define UTIL_FOREACH_GENERIC(begin, end, as) \
	for (util::foreach_helper<decltype((begin)), decltype((end))> _util_fe_{(begin), (end), 1}; _util_fe_.clause(); _util_fe_.act()) \
		for (auto as = *_util_fe_.it; !_util_fe_.cont; _util_fe_.cont = 1)

#define UTIL_FOREACH(c, as) UTIL_FOREACH_GENERIC(util::begin((c)), util::end((c)), as)
#define UTIL_CFOREACH(c, as) UTIL_FOREACH_GENERIC(util::cbegin((c)),util::cend((c)), as)
#define UTIL_RANGE_FOREACH(begin, end, as) UTIL_FOREACH_GENERIC(begin, end, as)

#define UTIL_IFOREACH(c, as) for (auto as = util::begin((c)); as != util::end((c)); ++as)
#define UTIL_CIFOREACH(c, as) for (auto as = util::cbegin((c)); as != util::cend((c)); ++as)
#define UTIL_RANGE_IFOREACH(begin, end, as) for (auto as = (begin); as != (end); ++as)

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
 * Parse a string time period to a number
 * @param timestr amount of time in a human readable format (eg. 2h30m)
 * @return number of seconds
 */
double tdparse(std::string timestr);

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
static inline int path_length(int x1, int y1, int x2, int y2)
{
	int dx = std::abs(x1 - x2);
	int dy = std::abs(y1 - y2);

	return dx + dy;
}

}

#endif // UTIL_HPP_INCLUDED
