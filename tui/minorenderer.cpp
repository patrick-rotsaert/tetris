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

void MinoRenderer::render(AsioTerminal& terminal, int x, int y, TetrominoColor color)
{
	terminal.print(mino_color(color), x, y, Character::GC_BLOCK);
}

void MinoRenderer::render(AsioTerminal& terminal, const Position& pos, TetrominoColor color)
{
	return render(terminal, pos.x, pos.y, color);
}

} // namespace tui
