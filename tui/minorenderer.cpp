#include "minorenderer.h"
#include "tetrominocolor.h"
#include "attribute.h"
#include "character.h"
#include "asioterminal.h"

#include <map>
#include <cassert>

namespace tui {

namespace {

using MinoColorsMapType = std::map<TetrominoColor, Attribute::type>;

const MinoColorsMapType& mino_color_map()
{
	static const MinoColorsMapType _{ { TetrominoColor::YELLOW, Attribute::FG_YELLOW },
		                              { TetrominoColor::RED, Attribute::FG_LIGHTRED },
		                              { TetrominoColor::PURPLE, Attribute::FG_LIGHTMAGENTA },
		                              { TetrominoColor::GREEN, Attribute::FG_LIGHTGREEN },
		                              { TetrominoColor::ORANGE, Attribute::FG_BROWN },
		                              { TetrominoColor::BLUE, Attribute::FG_LIGHTBLUE },
		                              { TetrominoColor::CYAN, Attribute::FG_CYAN } };
	return _;
}

Attribute::type mino_color(TetrominoColor color)
{
	auto it = mino_color_map().find(color);
	assert(it != mino_color_map().end());
	return it->second;
}

} // namespace

MinoRenderer::MinoRenderer()
    : size_{ 1, 1 }
{
}

MinoRenderer& MinoRenderer::instance()
{
	static MinoRenderer _{};
	return _;
}

void MinoRenderer::render(AsioTerminal& terminal, int x, int y, Attribute::type attr)
{
	for (int row = 0; row < this->size_.rows; ++row)
	{
		for (int column = 0; column < this->size_.colums; ++column)
		{
			terminal.print(attr, x + column, y + row, Character::GC_BLOCK);
		}
	}
}

void MinoRenderer::render(AsioTerminal& terminal, int x, int y, TetrominoColor color)
{
	return this->render(terminal, x, y, mino_color(color));
}

void MinoRenderer::render(AsioTerminal& terminal, const Position& pos, Attribute::type attr)
{
	return this->render(terminal, pos.x, pos.y, attr);
}

void MinoRenderer::render(AsioTerminal& terminal, const Position& pos, TetrominoColor color)
{
	return this->render(terminal, pos.x, pos.y, color);
}

const Size& MinoRenderer::size() const
{
	return this->size_;
}

void MinoRenderer::setSize(const Size& size)
{
	assert(size.colums > 0 && size.rows > 0);
	this->size_ = size;
}

} // namespace tui
