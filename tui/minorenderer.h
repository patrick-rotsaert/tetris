#pragma once

#include "position.h"

enum class TetrominoColor;

namespace tui {

class AsioTerminal;

class MinoRenderer final
{
public:
	static void render(AsioTerminal& terminal, int x, int y, TetrominoColor color);
	static void render(AsioTerminal& terminal, const Position& pos, TetrominoColor color);
};

} // namespace tui
