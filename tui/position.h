#pragma once

namespace tui {

struct Position
{
	int x, y;

	Position operator+(const Position& other) const
	{
		return Position{ this->x + other.x, this->y + other.y };
	}

	Position& operator+=(const Position& other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
};

} // namespace tui
