#pragma once

namespace tui {

struct Size
{
	int rows;
	int colums;

	bool operator<=(const Size& other) const
	{
		return this->rows <= other.rows && this->colums <= other.colums;
	}
};

} // namespace tui
