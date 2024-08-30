#pragma once

#include "asiotimer.h"
#include "keymodifier.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/signal_set.hpp>

#include <functional>
#include <array>
#include <vector>
#include <memory>
#include <cstdint>

namespace tui {

class AsioInput final
{
	static constexpr int     NOCHAR_TIMEOUT = 200;
	static constexpr int16_t NOCHAR         = -1;

	boost::asio::posix::stream_descriptor input_;
	AsioTimer                             timer_;
	std::function<void(int)>              keyPressedHandler_;

	std::array<char, 16> readBuffer_{};
	std::vector<int16_t> inputBuffer_{};

	std::unique_ptr<boost::asio::signal_set> ctrlC_;

	using Iterator = std::vector<int16_t>::iterator;

	void fail(std::string_view what, boost::system::error_code ec);

	void asyncRead();

	void onRead(boost::system::error_code ec, std::size_t length);

	void processInputBuffer();

	void processKey(int16_t key);

	void processEscapeSequence(Iterator begin, Iterator end);

	template<typename T>
	void keyPressed(const T& code)
	{
		this->keyPressedHandler_(static_cast<int>(code));
	}

	template<typename T>
	void keyPressed(const T& code, const std::vector<KeyModifier>& modifiers)
	{
		auto key = static_cast<int>(code);
		for (const auto m : modifiers)
		{
			key |= static_cast<int>(m);
		}
		this->keyPressedHandler_(key);
	}

public:
	explicit AsioInput(boost::asio::io_context& ioc, int fd, std::function<void(int)> keyPressedHandler, bool trapCtrlC);

	void setKeyPressedHandler(std::function<void(int key)> handler);
};

} // namespace tui
