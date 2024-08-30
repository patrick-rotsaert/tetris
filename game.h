#pragma once

#include <memory>
#include <functional>

class Board;
class ITimer;
enum class InputEvent;

class Game final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit Game(std::function<void()> onUpdate, std::unique_ptr<ITimer> timer);
	~Game() noexcept;

	void start();

	Board& board();

	void processInputEvent(InputEvent event);
};
