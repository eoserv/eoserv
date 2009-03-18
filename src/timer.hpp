#ifndef TIMER_HPP_INCLUDED
#define TIMER_HPP_INCLUDED

#include <list>

class Timer;
class TimeEvent;

typedef void(*TimerCallback)(void *);

class Timer
{
	protected:
		std::list<TimeEvent *> timers;

	public:
		static const int FOREVER = -1;

		static double GetTime();

		void Tick();

		void Register(TimeEvent *);
		void Unregister(TimeEvent *);
};

struct TimeEvent
{
	Timer *manager;
	TimerCallback callback;
	void *param;
	double speed;
	double lasttime;
	int lifetime;

	TimeEvent(TimerCallback callback, void *param, double speed, int lifetime = 1);

	~TimeEvent();
};

#endif // TIMER_HPP_INCLUDED
