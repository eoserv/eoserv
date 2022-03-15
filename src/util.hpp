
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <string>
#include <vector>

/**
 * Utility functions to assist with common tasks
 */
namespace util
{

using std::begin;
using std::end;
using std::cbegin;
using std::cend;

#define UTIL_RANGE(c) (util::begin((c))), (util::end((c)))
#define UTIL_CRANGE(c) (util::cbegin((c))), (util::cend((c)))

#define UTIL_FOREACH_REF(c, as) for (auto& as : (c))
#define UTIL_FOREACH_CREF(c, as) for (const auto& as : (c))

#define UTIL_FOREACH(c, as) for (const auto as : (c))

#define UTIL_IFOREACH(c, as) for (auto as = util::begin((c)); as != util::end((c)); ++as)
#define UTIL_CIFOREACH(c, as) for (auto as = util::cbegin((c)); as != util::cend((c)); ++as)

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
std::vector<std::string> explode(char delimiter, const std::string&);

/**
 * Split a string in to a vector with a specified delimiter
 */
std::vector<std::string> explode(const std::string& delimiter, const std::string&);

/**
 * Parse a string time period to a number
 * @param timestr amount of time in a human readable format (eg. 2h30m)
 * @return number of seconds
 */
double tdparse(const std::string& timestr);

int to_int(const std::string &);
unsigned int to_uint_raw(const std::string &);
double to_float(const std::string &);

std::string to_string(int);
std::string to_string(double);

std::string lowercase(const std::string&);

std::string uppercase(const std::string&);

std::string ucfirst(const std::string&);

int rand(int min, int max);
double rand(double min, double max);

double round(double);

std::string timeago(double time, double current_time);

void sleep(double seconds);

int text_width(const std::string&);
int text_max_word_width(const std::string&);
std::string text_cap(const std::string&, int width, const std::string& elipses = "...");
std::string text_word_wrap(const std::string&, int width);

/**
 * Finds the distance IN TILES between a pair of x,y coordinates
 */
static inline int path_length(int x1, int y1, int x2, int y2)
{
	int dx = std::abs(x1 - x2);
	int dy = std::abs(y1 - y2);

	return dx + dy;
}

template <class T> T clamp(const T& val, const T& min, const T& max)
{
	return std::min<T>(std::max<T>(val, min), max);
}

}

#endif // UTIL_HPP_INCLUDED
