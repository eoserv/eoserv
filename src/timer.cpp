
#include "timer.hpp"

#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/types.h>
#include <sys/time.h>
#endif // WIN32

#include <algorithm>

#include "util.hpp"

Timer::Timer()
{
	this->changed = true;
}

double Timer::GetTime()
{
#ifdef WIN32
#if _WIN32_WINNT >= 0x0600
	return static_cast<double>(GetTickCount64())/1000.0;
#else // _WIN32_WINNT >= 0x0600
	return static_cast<double>(GetTickCount())/1000.0;
#endif // _WIN32_WINNT >= 0x0600
#else // WIN32
	timeval gettime;
	gettimeofday(&gettime, 0);
	return static_cast<double>(gettime.tv_sec) + static_cast<double>(gettime.tv_usec)/1000000.0;
#endif // WIN32
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
