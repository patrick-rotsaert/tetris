#include "boardrenderer.h"
#include "minorenderer.h"
#include "asioterminal.h"
#include "size.h"
#include "position.h"
#include "character.h"
#include "grid.h"
#include "board.h"
#include "gridposition.h"
#include "tetromino.h"

#include <fmt/format.h>

namespace tui {

/*
 ############ ########
 #          # #      #
 #          # # @@@@ #
 #          # #      #
 #          # #      #
 #          # ########
 #          #
 #          # Level
 #          # 1
 #          #
 #          # Lines
 #          # 15
 #          #
 #          # Score
 #          # 123450
 #    @     #
 #    @     #
 #    @@    #
 #          #
 # @@       #
 #@@@@  @@@ #
 ############
 */

Size BoardRenderer::size()
{
	return Size{
		(1 + Grid::height() + 1), // border top + grid rows + border bottom
		(1 + Grid::width() + 1 + 1 + 1 + PREVIEW_WIDTH +
		 1), // border left + grid columns + border right + space + border left + preview columns + border right
	};
}

void BoardRenderer::render(const Board& board, AsioTerminal& terminal)
{
	terminal.cls(Attribute::BG_BLACK);

	const auto boardSize    = BoardRenderer::size();
	const auto terminalSize = terminal.size();
	if (boardSize.rows > terminalSize.rows || boardSize.colums > terminalSize.colums)
	{
		throw std::runtime_error{ fmt::format(
			"Terminal too small. Need at least {} rows and {} columns.", boardSize.rows, boardSize.colums) };
	}

	Position boardOrigin{ (terminalSize.colums - boardSize.colums) / 2, (terminalSize.rows - boardSize.rows) / 2 };
	auto     origin = boardOrigin;

	const auto borderAttr = Attribute::FG_LIGHTGRAY;

	// vertical borders
	for (int row = 0; row < Grid::height() + 2; ++row)
	{
		// Left
		terminal.print(borderAttr, Position{ 0, row } + origin, Character::GC_CKBOARD);
		// Right
		terminal.print(borderAttr, Position{ 1 + Grid::width(), row } + origin, Character::GC_CKBOARD);
	}

	// horizontal borders
	for (int column = 0; column < Grid::width(); ++column)
	{
		// Top
		terminal.print(borderAttr, Position{ 1 + column, 0 } + origin, Character::GC_CKBOARD);
		// Bottom
		terminal.print(borderAttr, Position{ 1 + column, 1 + Grid::height() } + origin, Character::GC_CKBOARD);
	}

	// grid
	// shift the origin one mino down and right (i.e inside the border).
	origin += Position{ 1, 1 };
	for (int row = 0; row < Grid::height(); ++row)
	{
		for (int column = 0; column < Grid::width(); ++column)
		{
			const auto& cell = board.grid().cell(row, column);
			if (cell.has_value())
			{
				MinoRenderer::render(terminal, Position{ column, row } + origin, cell.value());
			}
		}
	}

	// Preview border
	origin = boardOrigin + Position{ (1 + Grid::width() + 1 + 1), 0 };
	// vertical borders
	for (int row = 0; row < PREVIEW_HEIGHT + 2; ++row)
	{
		// Left
		terminal.print(borderAttr, Position{ 0, row } + origin, Character::GC_CKBOARD);
		// Right
		terminal.print(borderAttr, Position{ 1 + PREVIEW_WIDTH, row } + origin, Character::GC_CKBOARD);
	}

	// horizontal borders
	for (int column = 0; column < PREVIEW_WIDTH; ++column)
	{
		// Top
		terminal.print(borderAttr, Position{ 1 + column, 0 } + origin, Character::GC_CKBOARD);
		// Bottom
		terminal.print(borderAttr, Position{ 1 + column, 1 + PREVIEW_HEIGHT } + origin, Character::GC_CKBOARD);
	}

	const auto& nextTetromino = board.nextTetromino();
	// shift the origin one mino down and right (i.e inside the border).
	origin += Position{ 1, 1 };
	const auto position = GridPosition{ 1, 1 };
	for (const auto& offs : nextTetromino.rotationState())
	{
		const auto row    = position.row + offs.y;
		const auto column = position.column + offs.x;
		MinoRenderer::render(terminal, Position{ column, row } + origin, nextTetromino.color());
	}

	const auto levelColor = Attribute::FG_YELLOW;
	const auto linesColor = Attribute::FG_LIGHTGREEN;
	const auto scoreColor = Attribute::FG_LIGHTCYAN;

	// Level
	origin = boardOrigin + Position{ (1 + Grid::width() + 1 + 1), (1 + PREVIEW_HEIGHT + 1 + 1 + 2) };
	terminal.print(levelColor, origin, "Level");

	origin += Position{ 0, 1 };
	terminal.print(levelColor, origin, fmt::format("{}", board.level()));

	// Lines
	origin += Position{ 0, 2 };
	terminal.print(linesColor, origin, "Lines");

	origin += Position{ 0, 1 };
	terminal.print(linesColor, origin, fmt::format("{}", board.lines()));

	// Score
	origin += Position{ 0, 2 };
	terminal.print(scoreColor, origin, "Score");

	origin += Position{ 0, 1 };
	terminal.print(scoreColor, origin, fmt::format("{}", board.score()));

	// GAME OVER
	if (board.gameOver())
	{
		const auto gameOverColor = Attribute::FG_LIGHTRED | Attribute::FG_BLINK;

		origin += Position{ 0, 3 };
		terminal.print(gameOverColor, origin, "G A M E");
		origin += Position{ 0, 2 };
		terminal.print(gameOverColor, origin, "O V E R");
	}

	terminal.update();
}

} // namespace tui
