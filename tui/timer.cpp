#include "timer.h"

namespace tui {

Timer::Timer(boost::asio::io_context& ioc)
    : timer_{ ioc }
{
}

void Timer::start(int msec, std::function<void()> callback)
{
	this->timer_.start(msec, callback);
}

void Timer::stop()
{
	this->timer_.cancel();
}

} // namespace tui
