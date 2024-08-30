#pragma once

#include "itimer.h"
#include "asiotimer.h"

#include <boost/asio/io_context.hpp>

#include <functional>

namespace tui {

class Timer final : public ITimer
{
	AsioTimer timer_;

public:
	explicit Timer(boost::asio::io_context& ioc);

	void start(int msec, std::function<void()> callback) override;
	void stop() override;
};

} // namespace tui
