
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "timer.hpp"

#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/time.h>
#include <sys/types.h>
#endif // WIN32

#include <algorithm>
#include <cmath>

#include "util.hpp"

int rres = 0;
double gettime_offset = 0.0;
double gettime_last = 0.0;
bool gettime_init = false;

Timer::Timer()
{
	int res;
#ifdef WIN32
#ifndef TIMER_GETTICKCOUNT
	for (res = 1; res <= 50; ++res)
	{
		if (timeBeginPeriod(res) == TIMERR_NOERROR)
		{
			rres = res;
			break;
		}
	}
#endif // TIMER_GETTICKCOUNT
#endif // WIN32
	double first = Timer::GetTime();
	double cur;
	double last = first;
	double sum = 0.0;

	for (int i = 0; i < 100; )
	{
		cur = Timer::GetTime();

		if (cur != last)
		{
			sum += cur - last;
			last = cur;
			++i;
		}
	};

	this->resolution = sum / 100.0 - first;

	this->changed = true;
}

double Timer::GetTime()
{
#ifdef WIN32
#ifdef TIMER_GETTICKCOUNT
	unsigned int ticks = GetTickCount();
#else // TIMER_GETTICKCOUNT
	unsigned int ticks = timeGetTime();
#endif // TIMER_GETTICKCOUNT
	double ms = double(ticks) / 1000.0;
#else // WIN32
#ifdef CLOCK_MONOTONIC
	struct timespec gettime;
	clock_gettime(CLOCK_MONOTONIC, &gettime);
	double ms = double(gettime.tv_sec) + double(gettime.tv_nsec) / 1000000000.0;
#else // CLOCK_MONOTONIC
	struct timeval gettime;
	gettimeofday(&gettime, 0);
	double ms = double(gettime.tv_sec) + double(gettime.tv_usec) / 1000000.0;
#endif // CLOCK_MONOTONIC
#endif // WIN32
	if (!gettime_init)
	{
		gettime_last = ms;
		gettime_init = true;
	}

	double relms = ms - gettime_last;
	gettime_last = ms;

	gettime_offset += relms;

	return gettime_offset;
}

void Timer::Tick()
{
	double currenttime = Timer::GetTime();

	if (this->changed)
	{
		this->execlist = this->timers;
	}

	std::set<TimeEvent *> container = this->execlist;
	UTIL_SET_FOREACH_ALL(this->execlist, TimeEvent *, timer)
	{
		if (timer->lasttime + timer->speed < currenttime)
		{
			timer->callback(timer->param);
			timer->lasttime += timer->speed;

			if (timer->lifetime != Timer::FOREVER)
			{
				--timer->lifetime;

				if (timer->lifetime == 0)
				{
					this->Unregister(timer);
				}
			}
		}
	}
}

void Timer::Register(TimeEvent *timer)
{
	this->changed = true;

	if (timer->lifetime == 0)
	{
		return;
	}

	timer->lasttime = Timer::GetTime();
	this->timers.insert(timer);
}

void Timer::Unregister(TimeEvent *timer)
{
	this->changed = true;
	this->timers.erase(timer);
	if (timer->autofree)
	{
		delete timer;
	}
}

Timer::~Timer()
{
	UTIL_SET_FOREACH_ALL(this->timers, TimeEvent *, timer)
	{
		if (timer->autofree)
		{
			delete timer;
		}
	}
#ifdef WIN32
	if (rres != 0)
	{
		timeEndPeriod(rres);
		rres = 0;
	}
#endif // WIN32
}

TimeEvent::TimeEvent(TimerCallback callback, void *param, double speed, int lifetime, bool autofree)
{
	this->callback = callback;
	this->param = param;
	this->speed = speed;
	this->lifetime = lifetime;
	this->manager = 0;
	this->autofree = autofree;
}

TimeEvent::~TimeEvent()
{
	if (this->manager != 0)
	{
		this->manager->Unregister(this);
	}
}
