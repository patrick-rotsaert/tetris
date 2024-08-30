#pragma once

#include <boost/asio/io_context.hpp>

#include <memory>
#include <functional>

namespace tui {

class AsioTimer final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit AsioTimer(boost::asio::io_context& ioc);
	~AsioTimer() noexcept;

	void start(int msec, std::function<void()> work);
	void cancel();
};

} // namespace tui
