/* console.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "console.hpp"

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <string>

#include "platform.h"

#ifdef WIN32
#include "eoserv_windows.h"
#endif // WIN32

namespace Console
{

bool Styled[2] = {true, true};

static int day = -1;

#ifdef WIN32

static HANDLE Handles[2];

inline void Init(Stream i)
{
	if (!Handles[i])
	{
		Handles[i] = GetStdHandle((i == STREAM_OUT) ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	}
}

void SetTextColor(Stream stream, Color color, bool bold)
{
	Init(stream);
	SetConsoleTextAttribute(Handles[stream], color + (bold ? 8 : 0));
}

void ResetTextColor(Stream stream)
{
	Init(stream);
	SetConsoleTextAttribute(Handles[stream], COLOR_GREY);
}

#else // WIN32

static const int ansi_color_map[] = {0, 4, 2, 6, 1, 5, 3, 7, 0};

void SetTextColor(Stream stream, Color color, bool bold)
{
	char command[8] = {27, '[', '1', ';', '3', '0', 'm', 0};

	if (bold)
	{
		command[5] += ansi_color_map[static_cast<int>(color)];
	}
	else
	{
		for (int i = 2; i < 6; ++i)
			command[i] = command[i + 2];

		command[3] += ansi_color_map[static_cast<int>(color)];
	}

	std::fputs(command, (stream == STREAM_OUT) ? stdout : stderr);
}

void ResetTextColor(Stream stream)
{
	char command[5] = {27, '[', '0', 'm', 0};
	std::fputs(command, (stream == STREAM_OUT) ? stdout : stderr);
}

#endif // WIN32

static void print_timestamp(FILE* fh)
{
	std::time_t rawtime;
	char timestr[256];
	std::time(&rawtime);
	struct std::tm* tm = std::localtime(&rawtime);

	int new_day = tm->tm_year * 1000 + tm->tm_yday;

	if (new_day != day)
	{
		day = new_day;
		std::strftime(timestr, 256, "%a %b %e %Y", tm);
		std::fprintf(fh, "\n\n--- %s ---\n\n", timestr);
	}

	std::strftime(timestr, 256, "%H:%M:%S ", tm);
	std::fputs(timestr, fh);
}

#define CONSOLE_GENERIC_OUT(prefix, stream, color, bold) \
do { \
	FILE* fh = (stream == STREAM_OUT) ? stdout : stderr; \
	if (Styled[stream]) SetTextColor(stream, COLOR_GREY, false); \
	print_timestamp(fh); \
	if (Styled[stream] && color != COLOR_GREY) SetTextColor(stream, color, bold); \
	va_list args; \
	va_start(args, f); \
	std::fputs("[" prefix "] ", fh); \
	std::vfprintf(fh, f, args); \
	std::fputs("\n", fh); \
	va_end(args); \
	if (Styled[stream]) ResetTextColor(stream); \
} while (false)

void Out(const char* f, ...)
{
	CONSOLE_GENERIC_OUT("   ", STREAM_OUT, COLOR_GREY, true);
}

void Wrn(const char* f, ...)
{
	CONSOLE_GENERIC_OUT("WRN", STREAM_OUT, COLOR_YELLOW, true);
}

void Err(const char* f, ...)
{
	if (!Styled[STREAM_ERR])
	{
		CONSOLE_GENERIC_OUT("ERR", STREAM_OUT, COLOR_RED, true);
	}

	CONSOLE_GENERIC_OUT("ERR", STREAM_ERR, COLOR_RED, true);
}

void Dbg(const char* f, ...)
{
	CONSOLE_GENERIC_OUT("DBG", STREAM_OUT, COLOR_GREY, false);
}

}
