
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "database.hpp"
#include "timer.hpp"
#include "socket.hpp"

#include "util.hpp"

#include "platform.h"

#include <ctime>
#include <stdexcept>

#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/time.h>
#include <sys/types.h>
#endif // WIN32

#include <pthread.h>

#include "console.hpp"

static int rres = 0;

static unsigned int clock_ticks()
{
#ifdef WIN32
#ifdef TIMER_GETTICKCOUNT
	unsigned int ticks = GetTickCount();
#else // TIMER_GETTICKCOUNT
	unsigned int ticks = timeGetTime();
#endif // TIMER_GETTICKCOUNT
#else // WIN32
#ifdef CLOCK_MONOTONIC
	struct timespec gettime;
	clock_gettime(CLOCK_MONOTONIC, &gettime);
	std::time_t sec = gettime.tv_sec;
	long msec = gettime.tv_nsec / 1000000;
#else // CLOCK_MONOTONIC
	struct timeval gettime;
	gettimeofday(&gettime, 0);
	std::time_t sec = gettime.tv_sec;
	long msec = gettime.tv_usec / 1000;
#endif // CLOCK_MONOTONIC
	unsigned int ticks = static_cast<unsigned int>(sec * 1000 + msec);
#endif // WIN32

	return ticks;
}

Clock::Clock(int max_delta)
	: offset(0.0)
	, last(clock_ticks())
	, max_delta(1000)
{
	SetMaxDelta(max_delta);
}

unsigned int Clock::GetTimeDelta()
{
	unsigned int ticks = clock_ticks();
	unsigned int delta = ticks - last;

	if ((int)delta < 0)
	{
		Console::Wrn("A time delta of %i ms was detected and ignored.", delta);
		delta = 0;
	}

	if ((int)delta > max_delta)
	{
		Console::Wrn("A time delta of %i ms was detected and ignored.", delta);
		delta = max_delta;
	}

	last = ticks;

	return delta;
}

double Clock::GetTime()
{
	double relms = double(this->GetTimeDelta()) / 1000.0;
	offset += relms;
	return offset;
}

void Clock::SetMaxDelta(int max_delta)
{
	if (max_delta < 1)
	{
		Console::Wrn("Invalid clock max delta. Defaulting to 1000 ms.");
		max_delta = 1000;
	}

	this->max_delta = max_delta;
}

std::unique_ptr<Clock> Timer::clock;

struct Timer::impl_t
{
	pthread_mutex_t m;

	impl_t()
		: m(PTHREAD_MUTEX_INITIALIZER)
	{
		if (pthread_mutex_init(&m, 0) != 0)
			throw std::runtime_error("Timer mutex init failed");
	}

	void lock()
	{
		pthread_mutex_lock(&m);
	}

	void unlock()
	{
		pthread_mutex_unlock(&m);
	}

	~impl_t()
	{
		pthread_mutex_destroy(&m);
	}
};

Timer::Timer()
	: impl(new impl_t)
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
	if (!clock)
		clock.reset(new Clock());

	return clock->GetTime();
}

void Timer::SetMaxDelta(int max_delta)
{
	if (!clock)
		clock.reset(new Clock(max_delta));
	else
		clock->SetMaxDelta(max_delta);
}

void Timer::Tick()
{
	double currenttime = Timer::GetTime();

	impl->lock();
	if (this->changed)
	{
		this->execlist = this->timers;
		this->changed = false;
	}

	UTIL_FOREACH(this->execlist, timer)
	{
		if (this->timers.find(timer) == this->timers.end())
			continue;

		if (timer->lasttime + timer->speed < currenttime)
		{
			impl->unlock();
			timer->lasttime += timer->speed;

			if (timer->lifetime != Timer::FOREVER)
			{
				--timer->lifetime;

				if (timer->lifetime == 0)
				{
					this->Unregister(timer);
				}
			}

#ifndef DEBUG_EXCEPTIONS
			try
			{
#endif // DEBUG_EXCEPTIONS
				timer->callback(timer->param);
#ifndef DEBUG_EXCEPTIONS
			}
			catch (Socket_Exception& e)
			{
				Console::Err("Timer callback caused an exception");
				Console::Err("%s: %s", e.what(), e.error());
			}
			catch (Database_Exception& e)
			{
				Console::Err("Timer callback caused an exception");
				Console::Err("%s: %s", e.what(), e.error());
			}
			catch (std::runtime_error& e)
			{
				Console::Err("Timer callback caused an exception");
				Console::Err("Runtime Error: %s", e.what());
			}
			catch (std::logic_error& e)
			{
				Console::Err("Timer callback caused an exception");
				Console::Err("Logic Error: %s", e.what());
			}
			catch (std::exception& e)
			{
				Console::Err("Timer callback caused an exception");
				Console::Err("Uncaught Exception: %s", e.what());
			}
			catch (...)
			{
				Console::Err("Timer callback caused an exception");
			}
#endif // DEBUG_EXCEPTIONS

			if (timer->manager == 0)
				delete timer;

			impl->lock();
		}
	}

	impl->unlock();
}

void Timer::Register(TimeEvent *timer)
{
	if (timer->lifetime == 0)
	{
		return;
	}

	timer->lasttime = Timer::GetTime();
	timer->manager = this;

	impl->lock();
	this->changed = true;
	this->timers.insert(timer);
	impl->unlock();
}

void Timer::Unregister(TimeEvent *timer)
{
	impl->lock();
	this->changed = true;
	this->timers.erase(timer);
	impl->unlock();
	timer->manager = 0;
}

Timer::~Timer()
{
	impl->lock();
	UTIL_FOREACH(this->timers, timer)
	{
		timer->manager = 0;
		delete timer;
	}
	this->timers.clear();
	impl->unlock();

#ifdef WIN32
	if (rres != 0)
	{
		timeEndPeriod(rres);
		rres = 0;
	}
#endif // WIN32
}

TimeEvent::TimeEvent(TimerCallback callback, void *param, double speed, int lifetime)
{
	this->callback = callback;
	this->param = param;
	this->speed = speed;
	this->lifetime = lifetime;
	this->manager = 0;
}

TimeEvent::~TimeEvent()
{
	if (this->manager != 0)
	{
		this->manager->Unregister(this);
	}
}
