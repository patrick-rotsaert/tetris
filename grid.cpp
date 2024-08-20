#include "grid.h"
#include "gridposition.h"
#include "tetromino.h"

#include <cassert>

bool Grid::rowIsFull(int row) const
{
	for (int column = 0; column < this->width(); ++column)
	{
		if (!this->cell(row, column).has_value())
		{
			return false;
		}
	}
	return true;
}

void Grid::deleteRow(int row)
{
	for (; row > 0; --row)
	{
		this->cells_[row] = this->cells_[row - 1];
	}
	assert(row == 0);
	for (int column = 0; column < this->width(); ++column)
	{
		this->cell(row, column) = std::nullopt;
	}
}

Grid::Grid()
{
}

Grid::Cell& Grid::cell(int row, int column)
{
	assert(row > -1 && row < HEIGHT && column > -1 && column < WIDTH);
	return this->cells_[row][column];
}

const Grid::Cell& Grid::cell(int row, int column) const
{
	assert(row > -1 && row < HEIGHT && column > -1 && column < WIDTH);
	return this->cells_[row][column];
}

bool Grid::accepts(const Tetromino& tetromino, const GridPosition& position) const
{
	for (const auto& offs : tetromino.rotationState())
	{
		const auto row    = position.row + offs.y;
		const auto column = position.column + offs.x;
		if (column < 0 || column >= WIDTH || row < 0 || row >= HEIGHT || this->cells_[row][column])
		{
			return false;
		}
	}
	return true;
}

int Grid::clearFullLines()
{
	int linesCleared{};
	for (int row = this->height() - 1; row >= 0;)
	{
		if (this->rowIsFull(row))
		{
			this->deleteRow(row);
			++linesCleared;
		}
		else
		{
			--row;
		}
	}
	return linesCleared;
}
