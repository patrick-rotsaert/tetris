#include "asiotimer.h"

#include <boost/asio/deadline_timer.hpp>

namespace tui {

class AsioTimer::impl
{
	boost::asio::deadline_timer deadline_;
	bool                        cancelled_;

	void setDeadline(int msec)
	{
		this->deadline_.expires_from_now(boost::posix_time::milliseconds{ msec });
	}

public:
	explicit impl(boost::asio::io_context& ioc)
	    : deadline_{ ioc }
	    , cancelled_{}
	{
	}

	template<typename T>
	void start(const T& time, std::function<void()> work)
	{
		this->deadline_.cancel();
		this->cancelled_ = false;
		this->setDeadline(time);
		this->deadline_.async_wait([this, work](boost::system::error_code ec) {
			if (!ec)
			{
				if (!this->cancelled_)
				{
					if (this->deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
					{
						work();
					}
				}
			}
		});
	}

	void cancel()
	{
		this->cancelled_ = true;
		this->deadline_.cancel();
	}
};

AsioTimer::AsioTimer(boost::asio::io_context& ioc)
    : pimpl_(std::make_unique<impl>(ioc))
{
}

AsioTimer::~AsioTimer() noexcept
{
}

void AsioTimer::start(int msec, std::function<void()> work)
{
	pimpl_->start(msec, work);
}

void AsioTimer::cancel()
{
	pimpl_->cancel();
}

} // namespace tui
