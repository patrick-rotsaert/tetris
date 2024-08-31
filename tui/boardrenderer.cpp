#include "boardrenderer.h"
#include "minorenderer.h"
#include "asioterminal.h"
#include "size.h"
#include "position.h"
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
	const auto& minoSize = MinoRenderer::instance().size();
	return Size{
		(1 + Grid::height() + 1) * minoSize.rows, // border top + grid rows + border bottom
		(1 + Grid::width() + 1 + 1 + 1 + PREVIEW_WIDTH + 1) *
		    minoSize.colums, // border left + grid columns + border right + space + border left + preview columns + border right
	};
}

void BoardRenderer::render(const Board& board, AsioTerminal& terminal)
{
	terminal.cls(Attribute::BG_BLACK);
	const auto terminalSize = terminal.size();

	auto& minoRenderer = MinoRenderer::instance();

	Size               boardSize{};
	std::optional<int> minoHeight{};
	for (int height = 1;; ++height)
	{
		minoRenderer.setSize(Size{ height, height * 2 });
		boardSize = BoardRenderer::size();
		if (boardSize <= terminalSize)
		{
			minoHeight = height;
		}
		else
		{
			break;
		}
	}

	if (minoHeight)
	{
		minoRenderer.setSize(Size{ minoHeight.value(), minoHeight.value() * 2 });
		boardSize = BoardRenderer::size();
	}
	else
	{
		throw std::runtime_error{ fmt::format(
			"Terminal too small. Need at least {} rows and {} columns.", boardSize.rows, boardSize.colums) };
	}

	Position boardOrigin{ (terminalSize.colums - boardSize.colums) / 2, (terminalSize.rows - boardSize.rows) / 2 };
	auto     origin = boardOrigin;

	const auto  borderAttr = Attribute::FG_LIGHTGRAY;
	const auto& minoSize   = minoRenderer.size();

	// vertical borders
	for (int row = 0; row < Grid::height() + 2; ++row)
	{
		// Left
		minoRenderer.render(terminal, Position{ 0, row * minoSize.rows } + origin, borderAttr);
		// Right
		minoRenderer.render(terminal, Position{ (1 + Grid::width()) * minoSize.colums, row * minoSize.rows } + origin, borderAttr);
	}

	// horizontal borders
	for (int column = 0; column < Grid::width(); ++column)
	{
		// Top
		minoRenderer.render(terminal, Position{ (1 + column) * minoSize.colums, 0 } + origin, borderAttr);
		// Bottom
		minoRenderer.render(
		    terminal, Position{ (1 + column) * minoSize.colums, (1 + Grid::height()) * minoSize.rows } + origin, borderAttr);
	}

	// grid
	// shift the origin one mino down and right (i.e inside the border).
	origin += Position{ minoSize.colums, minoSize.rows };
	for (int row = 0; row < Grid::height(); ++row)
	{
		for (int column = 0; column < Grid::width(); ++column)
		{
			const auto& cell = board.grid().cell(row, column);
			if (cell.has_value())
			{
				minoRenderer.render(terminal, Position{ column * minoSize.colums, row * minoSize.rows } + origin, cell.value());
			}
		}
	}

	// Preview border
	origin = boardOrigin + Position{ (1 + Grid::width() + 1 + 1) * minoSize.colums, 0 };
	// vertical borders
	for (int row = 0; row < PREVIEW_HEIGHT + 2; ++row)
	{
		// Left
		minoRenderer.render(terminal, Position{ 0, row * minoSize.rows } + origin, borderAttr);
		// Right
		minoRenderer.render(terminal, Position{ (1 + PREVIEW_WIDTH) * minoSize.colums, row * minoSize.rows } + origin, borderAttr);
	}

	// horizontal borders
	for (int column = 0; column < PREVIEW_WIDTH; ++column)
	{
		// Top
		minoRenderer.render(terminal, Position{ (1 + column) * minoSize.colums, 0 } + origin, borderAttr);
		// Bottom
		minoRenderer.render(
		    terminal, Position{ (1 + column) * minoSize.colums, (1 + PREVIEW_HEIGHT) * minoSize.rows } + origin, borderAttr);
	}

	const auto& nextTetromino = board.nextTetromino();
	// shift the origin one mino down and right (i.e inside the border).
	origin += Position{ minoSize.colums, minoSize.rows };
	const auto position = GridPosition{ 1, 1 };
	for (const auto& offs : nextTetromino.rotationState())
	{
		const auto row    = position.row + offs.y;
		const auto column = position.column + offs.x;
		minoRenderer.render(terminal, Position{ column * minoSize.colums, row * minoSize.rows } + origin, nextTetromino.color());
	}

	const auto levelColor = Attribute::FG_YELLOW;
	const auto linesColor = Attribute::FG_LIGHTGREEN;
	const auto scoreColor = Attribute::FG_LIGHTCYAN;

	// Level
	origin = boardOrigin + Position{ (1 + Grid::width() + 1 + 1) * minoSize.colums, (1 + PREVIEW_HEIGHT + 1 + 1 + 2) * minoSize.rows };
	terminal.print(levelColor, origin, "Level");

	origin += Position{ 0, 1 * minoSize.rows };
	terminal.print(levelColor, origin, fmt::format("{}", board.level()));

	// Lines
	origin += Position{ 0, 2 * minoSize.rows };
	terminal.print(linesColor, origin, "Lines");

	origin += Position{ 0, 1 * minoSize.rows };
	terminal.print(linesColor, origin, fmt::format("{}", board.lines()));

	// Score
	origin += Position{ 0, 2 * minoSize.rows };
	terminal.print(scoreColor, origin, "Score");

	origin += Position{ 0, 1 * minoSize.rows };
	terminal.print(scoreColor, origin, fmt::format("{}", board.score()));

	// GAME OVER
	if (board.gameOver())
	{
		const auto gameOverColor = Attribute::FG_LIGHTRED | Attribute::FG_BLINK;

		origin += Position{ 0, 3 * minoSize.rows };
		terminal.print(gameOverColor, origin, "G A M E");
		origin += Position{ 0, 2 * minoSize.rows };
		terminal.print(gameOverColor, origin, "O V E R");
	}

	terminal.update();
}

} // namespace tui
