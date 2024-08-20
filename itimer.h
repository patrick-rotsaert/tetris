#pragma once

#include <functional>

class ITimer
{
public:
	virtual ~ITimer() noexcept;

	virtual void start(int msec, std::function<void()> callback) = 0;
	virtual void stop()                                          = 0;
};
