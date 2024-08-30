#pragma once

class Board;

namespace tui {

class Size;
class AsioTerminal;

class BoardRenderer final
{
	static constexpr int PREVIEW_WIDTH  = 6;
	static constexpr int PREVIEW_HEIGHT = 4;

public:
	static Size size();
	static void render(const Board& board, AsioTerminal& terminal);
};

} // namespace tui
