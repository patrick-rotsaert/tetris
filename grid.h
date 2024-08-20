#pragma once

#include <optional>
#include <array>

class Tetromino;
class GridPosition;
enum class TetrominoColor;

class Grid final
{
	static constexpr int WIDTH  = 10;
	static constexpr int HEIGHT = 20;

	using Cell = std::optional<TetrominoColor>;

	std::array<std::array<Cell, WIDTH>, HEIGHT> cells_{};

	bool rowIsFull(int row) const;

	void deleteRow(int row);

public:
	Grid();

	static constexpr int width()
	{
		return WIDTH;
	}

	static constexpr int height()
	{
		return HEIGHT;
	}

	Cell&       cell(int row, int column);
	const Cell& cell(int row, int column) const;

	bool accepts(const Tetromino& tetromino, const GridPosition& position) const;

	int clearFullLines();
};
