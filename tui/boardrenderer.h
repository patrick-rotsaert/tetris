#pragma once

class Board;

namespace tui {

class Size;
class AsioTerminal;

class BoardRenderer final
{
public:
	static constexpr int PREVIEW_WIDTH  = 6;
	static constexpr int PREVIEW_HEIGHT = 4;

	void calculateAndSetMinoSize(const AsioTerminal& terminal);
	void render(const Board& board, AsioTerminal& terminal);
};

} // namespace tui
