#include "asioinput.h"
#include "keycode.h"
#include "keymodifier.h"

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <optional>
#include <string_view>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>

namespace tui {

namespace {

template<typename Iterator>
auto dump(Iterator begin, Iterator end)
{
	fmt::memory_buffer buf{};
	for (auto it = begin; it < end; ++it)
	{
		const auto c = *it;
		if (c == 27)
		{
			fmt::format_to(std::back_inserter(buf), "ESC ");
		}
		else if (c == -1)
		{
			fmt::format_to(std::back_inserter(buf), "NOCHAR ");
		}
		else if (std::isprint(c))
		{
			fmt::format_to(std::back_inserter(buf), "'{:c}' ", c);
		}
		else
		{
			fmt::format_to(std::back_inserter(buf), "{:d} ", c & 0xff);
		}
	}
	return fmt::to_string(buf);
}

template<typename Iterator>
std::optional<std::vector<int>> parse_number_list(Iterator begin, Iterator end)
{
	auto result = std::vector<int>{};
	enum
	{
		NONE,
		NUMBER,
		SEMICOLON
	} state = NONE;
	int number{};
	for (auto it = begin; it < end; ++it)
	{
		const auto c = *it;
		if (c == ';')
		{
			switch (state)
			{
			case NONE:
				SPDLOG_WARN("Unexpected initial ';");
				return std::nullopt;
			case NUMBER:
				state = SEMICOLON;
				result.push_back(number);
				number = 0;
				break;
			case SEMICOLON:
				SPDLOG_WARN("Unexpected consecutive ';");
				return std::nullopt;
			}
		}
		else if (std::isdigit(c))
		{
			state = NUMBER;
			number *= 10;
			number += (c - '0');
		}
	}
	if (state == NUMBER)
	{
		result.push_back(number);
	}
	return result;
}

std::vector<KeyModifier> translate_key_modifiers(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end)
{
	auto result = std::vector<KeyModifier>{};
	for (auto it = begin; it < end; ++it)
	{
		auto number = *it;
		if (number < 1)
		{
			SPDLOG_WARN("Invalid modifier {}", number);
			continue;
		}
		--number;
		if (number & 1)
		{
			result.push_back(KeyModifier::SHIFT);
		}
		if (number & 2)
		{
			result.push_back(KeyModifier::ALT);
		}
		if (number & 4)
		{
			result.push_back(KeyModifier::CONTROL);
		}
		if (number & 8)
		{
			result.push_back(KeyModifier::META);
		}
	}
	return result;
}

} // namespace

void AsioInput::fail(std::string_view what, boost::system::error_code ec)
{
	SPDLOG_ERROR("{}: {}", what, ec.message());
}

void AsioInput::asyncRead()
{
	this->input_.async_read_some(boost::asio::buffer(this->readBuffer_),
	                             std::bind(&AsioInput::onRead, this, std::placeholders::_1, std::placeholders::_2));
}

void AsioInput::onRead(boost::system::error_code ec, std::size_t length)
{
	if (ec)
	{
		return this->fail("read", ec);
	}

	SPDLOG_TRACE("read: {}", dump(this->readBuffer_.begin(), this->readBuffer_.begin() + length));

	for (std::size_t i = 0; i < length; ++i)
	{
		this->inputBuffer_.push_back(static_cast<int16_t>(this->readBuffer_[i]) & 0xff);
	}

	this->timer_.cancel();

	this->asyncRead();

	this->processInputBuffer();
}

void AsioInput::processInputBuffer()
{
	auto&& erase = [this](int count) -> void { this->inputBuffer_.erase(this->inputBuffer_.begin(), this->inputBuffer_.begin() + count); };

	auto&& startTimer = [this]() -> void {
		this->timer_.start(NOCHAR_TIMEOUT, [this]() {
			this->inputBuffer_.push_back(NOCHAR);
			this->processInputBuffer();
		});
	};

	SPDLOG_TRACE("buff: {}", dump(this->inputBuffer_.begin(), this->inputBuffer_.end()));

	while (!this->inputBuffer_.empty())
	{
		const auto first = this->inputBuffer_.at(0u);
		if (first == 27)
		{
			if (this->inputBuffer_.size() > 1)
			{
				const auto second = this->inputBuffer_.at(1u);
				if (second == NOCHAR)
				{
					// { <esc> <nochar> } -> esc
					this->keyPressed(KeyCode::ESC);
					erase(2);
				}
				else if (second == 27)
				{
					// { <esc> } <esc> -> esc
					this->keyPressed(KeyCode::ESC);
					erase(1);
				}
				else if (second == '[' || second == 'O')
				{
					if (this->inputBuffer_.size() > 2)
					{
						const auto third = this->inputBuffer_.at(2u);
						if (third == NOCHAR)
						{
							// { <esc> [ <nochar> } -> Alt+[
							// { <esc> O <nochar> } -> Alt+O
							this->keyPressed(second, { KeyModifier::ALT });
							erase(3);
						}
						else if (third == 27)
						{
							// { <esc> [ } <esc> -> Alt+[
							// { <esc> O } <esc> -> Alt+O
							this->keyPressed(second, { KeyModifier::ALT });
							erase(2);
						}
						else
						{
							auto begin = this->inputBuffer_.begin() + 2;
							if (third == '[' && second == '[')
							{
								++begin;
							}
							const auto it = std::find_if(
							    begin, this->inputBuffer_.end(), [](int16_t c) { return (c >= 0x40 && c <= 0x7e) || c == NOCHAR; });
							if (it == this->inputBuffer_.end())
							{
								return startTimer();
							}
							else if (*it == NOCHAR)
							{
								this->processEscapeSequence(this->inputBuffer_.begin(), it);
								this->inputBuffer_.erase(this->inputBuffer_.begin(), it + 1);
							}
							else
							{
								this->processEscapeSequence(this->inputBuffer_.begin(), it + 1);
								this->inputBuffer_.erase(this->inputBuffer_.begin(), it + 1);
							}
						}
					}
					else
					{
						return startTimer();
					}
				}
				else
				{
					// <esc> <char> is Alt+char
					this->keyPressed(second, { KeyModifier::ALT });
					erase(2);
				}
			}
			else
			{
				return startTimer();
			}
		}
		else
		{
			this->processKey(first);
			erase(1);
		}
	}
}

void AsioInput::processKey(int16_t key)
{
	// BACKSPACE and Ctrl+H both generated ASCII 8
	if (key == 8 || key == 127)
	{
		return this->keyPressed(KeyCode::BACKSPACE);
	}

	// TAB and Ctrl+I both generated ASCII 9
	if (key == '\t')
	{
		return this->keyPressed(KeyCode::TAB);
	}

	// ENTER and Ctrl+J both generated ASCII 10
	if (key == '\n')
	{
		return this->keyPressed(KeyCode::ENTER);
	}

	// Ctrl+] -> 29
	if (key == 29)
	{
		return this->keyPressed(']', { KeyModifier::CONTROL });
	}

	// Ctrl+A -> 1
	// Ctrl+B -> 2
	// ...
	// Ctrl-Z -> 26
	if (key >= ('A' - 'A' + 1) && key <= ('Z' - 'A' + 1))
	{
		this->keyPressed('A' + key - 1, { KeyModifier::CONTROL });
	}
	else
	{
		this->keyPressed(key);
	}
}

void AsioInput::processEscapeSequence(Iterator begin, Iterator end)
{
	assert(std::distance(begin, end) >= 3);
	assert(*begin == 27);

	const auto second = *(begin + 1);
	assert(second == '[' || second == 'O');

	const auto third = *(begin + 2);

	SPDLOG_TRACE("seq: {}", dump(begin, end));

	const auto optNumbers = parse_number_list(begin + 2, end - 1);
	if (!optNumbers.has_value())
	{
		SPDLOG_WARN(
		    "Invalid escape sequence {}\n"
		    "Invalid number list.",
		    dump(begin, end));
		return;
	}
	const auto& numbers = optNumbers.value();
	SPDLOG_TRACE("numbers: {}", numbers);

	const auto last = *(end - 1);

	// Linux console generates ESC [ [ LETTER for F1 to F5
	if (second == '[' && third == '[' && std::isalpha(last))
	{
		auto keyCode = KeyCode{};
		switch (last)
		{
		case 'A':
			keyCode = KeyCode::F1;
			break;
		case 'B':
			keyCode = KeyCode::F2;
			break;
		case 'C':
			keyCode = KeyCode::F3;
			break;
		case 'D':
			keyCode = KeyCode::F4;
			break;
		case 'E':
			keyCode = KeyCode::F5;
			break;
		default:
			SPDLOG_WARN("Unknown console key code '{}'", last);
			return;
		}
		this->keyPressed(keyCode, translate_key_modifiers(numbers.begin(), numbers.end()));
	}
	// VT sequences
	else if (second == '[' && last == '~')
	{
		if (numbers.empty())
		{
			SPDLOG_WARN(
			    "Invalid escape sequence {}\n"
			    "At least one number expected.",
			    dump(begin, end));
			return;
		}
		auto keyCode = KeyCode{};
		switch (numbers.front())
		{
		case 1:
			keyCode = KeyCode::HOME;
			break;
		case 2:
			keyCode = KeyCode::INS;
			break;
		case 3:
			keyCode = KeyCode::DEL;
			break;
		case 4:
			keyCode = KeyCode::END;
			break;
		case 5:
			keyCode = KeyCode::PGUP;
			break;
		case 6:
			keyCode = KeyCode::PGDOWN;
			break;
		case 7:
			keyCode = KeyCode::HOME;
			break;
		case 8:
			keyCode = KeyCode::END;
			break;
		case 10:
			keyCode = KeyCode::F0;
			break;
		case 11:
			keyCode = KeyCode::F1;
			break;
		case 12:
			keyCode = KeyCode::F2;
			break;
		case 13:
			keyCode = KeyCode::F3;
			break;
		case 14:
			keyCode = KeyCode::F4;
			break;
		case 15:
			keyCode = KeyCode::F5;
			break;
		case 17:
			keyCode = KeyCode::F6;
			break;
		case 18:
			keyCode = KeyCode::F7;
			break;
		case 19:
			keyCode = KeyCode::F8;
			break;
		case 20:
			keyCode = KeyCode::F9;
			break;
		case 21:
			keyCode = KeyCode::F10;
			break;
		case 23:
			keyCode = KeyCode::F11;
			break;
		case 24:
			keyCode = KeyCode::F12;
			break;
		case 25:
			keyCode = KeyCode::F13;
			break;
		case 26:
			keyCode = KeyCode::F14;
			break;
		case 28:
			keyCode = KeyCode::F15;
			break;
		case 29:
			keyCode = KeyCode::F16;
			break;
		case 31:
			keyCode = KeyCode::F17;
			break;
		case 32:
			keyCode = KeyCode::F18;
			break;
		case 33:
			keyCode = KeyCode::F19;
			break;
		case 34:
			keyCode = KeyCode::F20;
			break;
		default:
			SPDLOG_WARN("Unknown VT key code {}", numbers.front());
			return;
		}
		this->keyPressed(keyCode, translate_key_modifiers(numbers.begin() + 1, numbers.end()));
	}
	// XTERM sequences
	else if ((second == '[' || second == 'O') && std::isupper(last))
	{
		auto keyCode = KeyCode{};
		switch (last)
		{
		case 'A':
			keyCode = KeyCode::UP;
			break;
		case 'B':
			keyCode = KeyCode::DOWN;
			break;
		case 'C':
			keyCode = KeyCode::RIGHT;
			break;
		case 'D':
			keyCode = KeyCode::LEFT;
			break;
		case 'F':
			keyCode = KeyCode::END;
			break;
		case 'G':
			keyCode = KeyCode::KEYPAD_5;
			break;
		case 'H':
			keyCode = KeyCode::HOME;
			break;
		case 'P':
			keyCode = KeyCode::F1;
			break;
		case 'Q':
			keyCode = KeyCode::F2;
			break;
		case 'R':
			keyCode = KeyCode::F3;
			break;
		case 'S':
			keyCode = KeyCode::F4;
			break;
		case 'Z':
			return this->keyPressed(KeyCode::TAB, { KeyModifier::SHIFT });
		default:
			SPDLOG_WARN("Unknown xterm key code '{}'", last);
			return;
		}
		this->keyPressed(keyCode, translate_key_modifiers(numbers.begin(), numbers.end()));
	}
	else
	{
		SPDLOG_WARN(
		    "Invalid escape sequence {}\n"
		    "Second-last combination not recognized.",
		    dump(begin, end));
	}
}

AsioInput::AsioInput(boost::asio::io_context& ioc, int fd, std::function<void(int)> keyPressedHandler, bool trapCtrlC)
    : input_{ ioc, fd }
    , timer_{ ioc }
    , keyPressedHandler_{ std::move(keyPressedHandler) }
    , ctrlC_{}
{
	this->asyncRead();

	if (trapCtrlC)
	{
		this->ctrlC_ = std::make_unique<boost::asio::signal_set>(ioc, SIGINT);
		this->ctrlC_->async_wait([&](const auto&, int) { this->keyPressed('C', { KeyModifier::CONTROL }); });
	}
}

void AsioInput::setKeyPressedHandler(std::function<void(int)> handler)
{
	this->keyPressedHandler_ = std::move(handler);
}

} // namespace tui
