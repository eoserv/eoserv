
#include "timer.hpp"

#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/types.h>
#include <sys/time.h>
#endif // WIN32

#include "util.hpp"

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
	UTIL_FOREACH(this->timers, timer)
	{
		if (timer->lifetime != 0 && timer->lasttime + timer->speed < currenttime)
		{
			timer->callback(timer->param);
			timer->lasttime += timer->speed;
			if (timer->lifetime != Timer::FOREVER)
			{
				--timer->lifetime;
			}
		}
	}
}

void Timer::Register(TimeEvent *timer)
{
	timer->lasttime = Timer::GetTime();
	this->timers.push_back(timer);
}

void Timer::Unregister(TimeEvent *timer)
{
	this->timers.remove(timer);
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
