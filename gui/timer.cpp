#include "timer.h"

namespace gui {

void Timer::timeout()
{
	if (this->callback_)
	{
		this->callback_();
	}
}

Timer::Timer()
{
	QObject::connect(&this->timer_, &QTimer::timeout, this, &Timer::timeout);
}

void Timer::start(int msec, std::function<void()> callback)
{
	this->callback_ = callback;
	this->timer_.start(msec);
}

void Timer::stop()
{
	this->timer_.stop();
}

} // namespace gui
