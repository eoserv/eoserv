
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "util.hpp"

#include <algorithm>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <limits>
#include <stdexcept>

#include "platform.h"

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include "util/variant.hpp"

namespace util
{

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

std::vector<std::string> explode(char delimiter, const std::string& str)
{
	std::size_t lastpos = 0;
	std::size_t pos = 0;
	std::vector<std::string> pieces;

	for (pos = str.find_first_of(delimiter); pos != std::string::npos; )
	{
		pieces.emplace_back(str.substr(lastpos, pos - lastpos));
		lastpos = pos+1;
		pos = str.find_first_of(delimiter, pos+1);
	}
	pieces.emplace_back(str.substr(lastpos));

	return pieces;
}

std::vector<std::string> explode(const std::string& delimiter, const std::string& str)
{
	std::size_t lastpos = 0;
	std::size_t pos = 0;
	std::vector<std::string> pieces;

	for (pos = str.find(delimiter); pos != std::string::npos; )
	{
		pieces.emplace_back(str.substr(lastpos, pos - lastpos));
		lastpos = pos + delimiter.length();
		pos = str.find(delimiter, pos + delimiter.length());
	}
	pieces.emplace_back(str.substr(lastpos));

	return pieces;
}

double tdparse(const std::string& timestr)
{
	static char period_names[] = {'s', 'm',  '%',   'k',    'h',    'd'    };
	static double period_mul[] = {1.0, 60.0, 100.0, 1000.0, 3600.0, 86400.0};
	double ret = 0.0;
	double val = 0.0;
	bool decimal = false;
	double decimalmulti = 0.1;
	bool negate = false;

	for (std::size_t i = 0; i < timestr.length(); ++i)
	{
		char c = timestr[i];
		bool found = false;

		if (c == '-')
		{
			negate = true;
			continue;
		}

		if (c >= 'A' && c <= 'Z')
		{
			c -= 'A' - 'a';
		}

		for (std::size_t ii = 0; ii < sizeof(period_names)/sizeof(char); ++ii)
		{
			if (c == period_names[ii])
			{
				if (c == 'm' && (i < timestr.length()-1 && timestr[i+1] == 's'))
				{
					ret += val / 1000.0;
					++i;
				}
				else if (c == '%')
				{
					ret += val / period_mul[ii];
				}
				else
				{
					ret += val * period_mul[ii];
				}

				found = true;
				val = 0.0;

				decimal = false;
				decimalmulti = 0.1;

				break;
			}
		}

		if (!found)
		{
			if (c >= '0' && c <= '9')
			{
				if (!decimal)
				{
					val *= 10.0;
					val += c - '0';
				}
				else
				{
					val += (c - '0') * decimalmulti;
					decimalmulti /= 10.0;
				}
			}
			else if (c == '.')
			{
				decimal = true;
				decimalmulti = 0.1;
			}
		}
	}

	return (ret + val) * (negate ? -1.0 : 1.0);
}

int to_int(const std::string &subject)
{
	return static_cast<int>(util::variant(subject));
}

unsigned int to_uint_raw(const std::string &subject)
{
	unsigned int multiplier = 1;
	unsigned int result = 0;

	std::size_t i = subject.length();

	if (i == 0)
		return 0;

	do {
		--i;

		if (subject[i] < '0' || subject[i] > '9')
		{
			throw std::invalid_argument("Non-numeric argument");
		}

		result += (subject[i] - '0') * multiplier;
		multiplier *= 10;
	} while (i > 0);

	return result;
}

double to_float(const std::string &subject)
{
	return static_cast<double>(util::variant(subject));
}

std::string to_string(int subject)
{
	return static_cast<std::string>(util::variant(subject));
}

std::string to_string(double subject)
{
	return static_cast<std::string>(util::variant(subject));
}

std::string lowercase(const std::string& subject)
{
	std::string result;
	result.resize(subject.length());

	std::transform(subject.begin(), subject.end(), result.begin(), static_cast<int(*)(int)>(std::tolower));

	return result;
}

std::string uppercase(const std::string& subject)
{
	std::string result;
	result.resize(subject.length());

	std::transform(subject.begin(), subject.end(), result.begin(), static_cast<int(*)(int)>(std::toupper));

	return result;
}

std::string ucfirst(const std::string& subject)
{
	std::string result(subject);

	if (!result.empty() && result[0] >= 'a' && result[0] <= 'z')
		result[0] += ('A' - 'a');

	return result;
}

struct rand_init
{
	rand_init()
	{
		static bool initialized;

		if (!initialized)
		{
			initialized = true;
			init();
		}
	}

	void init() const
	{
		std::srand(std::time(0));
	}
};

rand_init rand_init_instance;

static unsigned long long_rand()
{
#if RAND_MAX < 65535
	return (std::rand() & 0xFF) << 24 | (std::rand() & 0xFF) << 16 | (std::rand() & 0xFF) << 8 | (std::rand() & 0xFF);
#else
#if RAND_MAX < 4294967295
	return (std::rand() & 0xFFFF) << 16 | (std::rand() & 0xFFFF);
#else
	return std::rand() & 0xFFFFFFFFU;
#endif
#endif
}

int rand(int min, int max)
{
	return int(double(long_rand()) / 4294967296.0 * double(max - min + 1) + double(min));
}

double rand(double min, double max)
{
	return double(long_rand()) / 4294967296.0 * (max - min) + min;
}

double round(double subject)
{
	return std::floor(subject + 0.5);
}

std::string timeago(double time, double current_time)
{
	static bool init = false;
	static std::vector<std::pair<int, std::string>> times;

	if (!init)
	{
		init = true;
		times.resize(5);
		times.push_back(std::make_pair(1, "second"));
		times.push_back(std::make_pair(60, "minute"));
		times.push_back(std::make_pair(60*60, "hour"));
		times.push_back(std::make_pair(24*60*60, "day"));
		times.push_back(std::make_pair(7*24*60*60, "week"));
	}

	std::string ago;

	double diff = current_time - time;

	ago = ((diff >= 0) ? " ago" : " from now");

	diff = std::abs(diff);

	for (int i = times.size()-1; i >= 0; --i)
	{
		int x = int(diff / times[i].first);
		diff -= x * times[i].first;

		if (x > 0)
		{
			return util::to_string(x) + " " + times[i].second + ((x == 1) ? "" : "s") + ago;
		}
	}

	return "now";
}

void sleep(double seconds)
{
#ifdef WIN32
	Sleep(int(seconds * 1000.0));
#else // WIN32
	unsigned long sec = seconds;
	unsigned long nsec = (seconds - double(sec)) * 1000000000.0;
	timespec ts = {time_t(sec), long(nsec)};
	nanosleep(&ts, 0);
#endif // WIN32
}

static int sizes[256] = {
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, /* NUL -  SI */
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, /* DLE -  US */
	 3,  3,  5,  7,  6,  8,  6,  2,  3,  3,  4,  6,  3,  3,  3,  5, /* ' ' - '/' */
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  3,  3,  6,  6,  6,  6, /* '0' - '?' */
	11,  7,  7,  7,  8,  7,  6,  8,  8,  3,  5,  7,  6,  9,  8,  8, /* '@' - 'O' */
	 7,  8,  8,  7,  7,  8,  7, 11,  7,  7,  7,  3,  5,  3,  6,  6, /* 'P' - '_' */
	 3,  6,  6,  6,  6,  6,  3,  6,  6,  2,  2,  6,  2,  8,  6,  6, /* '`' - 'o' */
	 6,  6,  3,  5,  3,  6,  6,  8,  5,  5,  5,  4,  2,  4,  7,  3, /* 'p' - DEL */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 3,  3,  6,  6,  6,  6,  2,  6,  3,  9,  4,  6,  6,  3,  8,  6,
	 4,  6,  3,  3,  3,  6,  6,  3,  3,  3,  4,  6,  8,  8,  8,  6,
	 7,  7,  7,  7,  7,  7, 10,  7,  7,  7,  7,  7,  3,  3,  3,  3,
	 8,  8,  8,  8,  8,  8,  8,  6,  8,  8,  8,  8,  8,  7,  7,  6,
	 6,  6,  6,  6,  6,  6, 10,  6,  6,  6,  6,  6,  2,  4,  4,  4,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  6,  5,
};

int text_width(const std::string& string)
{
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		length += sizes[string[i] & 0xFF];
	}

	return length;
}

int text_max_word_width(const std::string& string)
{
	int max_length = 0;
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		if (string[i] == ' ')
		{
			max_length = std::max(max_length, length);
			length = 0;
		}
		else
		{
			length += sizes[string[i] & 0xFF];
		}
	}

	return std::max(max_length, length);
}

std::string text_cap(const std::string& string, int width, const std::string& elipses)
{
	int length = 0;

	for (std::size_t i = 0; i < string.length(); ++i)
	{
		length += sizes[string[i] & 0xFF];

		if (length > width)
		{
			int elipses_length = text_width(elipses);

			while (length > (width + elipses_length) && i > 0)
			{
				length -= string[i];
				--i;
			}

			return string.substr(0, i);
		}
	}

	return string;
}

std::string text_word_wrap(const std::string& string, int width)
{
	int length = 0;
	std::string result(string);

	for (std::size_t i = 0; i < result.length(); ++i)
	{
		if (result[i] == ' ')
		{
			length = 0;
		}
		else
		{
			length += sizes[result[i] & 0xFF];

			if (length > width)
			{
				result.insert(i - 1, " ");
				length = 0;
			}
		}
	}

	return result;
}

}
