#pragma once

#include "attribute.h"
#include "size.h"
#include "position.h"

#include <boost/asio/io_context.hpp>

#include <memory>
#include <functional>
#include <string_view>

namespace tui {

class AsioTerminal final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit AsioTerminal(boost::asio::io_context& ioc, std::function<void(int key)> keyPressedHandler = nullptr, bool trapCtrlC = false);
	~AsioTerminal() noexcept;

	// The value for `key` is a KeyCode enum (see keycode.h) or character,
	// possibly bitwised OR-er with one or more KeyModifier enums (see keymodifier.h).
	void setKeyPressedHandler(std::function<void(int key)> keyPressedHandler);

	Size size() const;
	void cursor(bool on);
	void cls(Attribute::type attr);
	void print(Attribute::type attr, int x, int y, std::string_view s);
	void print(Attribute::type attr, int x, int y, char c);
	void print(Attribute::type attr, const Position& pos, std::string_view s);
	void print(Attribute::type attr, const Position& pos, char c);

	void update();
};

} // namespace tui
