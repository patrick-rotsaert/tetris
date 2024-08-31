#pragma once

#include "position.h"
#include "size.h"
#include "attribute.h"

enum class TetrominoColor;

namespace tui {

class AsioTerminal;

class MinoRenderer final
{
	Size size_;

	MinoRenderer();

public:
	static MinoRenderer& instance();

	void render(AsioTerminal& terminal, int x, int y, Attribute::type attr);
	void render(AsioTerminal& terminal, int x, int y, TetrominoColor color);
	void render(AsioTerminal& terminal, const Position& pos, Attribute::type attr);
	void render(AsioTerminal& terminal, const Position& pos, TetrominoColor color);

	const Size& size() const;
	void        setSize(const Size& size);
};

} // namespace tui
