#pragma once

namespace tui {

// Graphic characters (values are as in DOS char set)
struct Character
{
	static constexpr auto GC_UARROW   = static_cast<char>(24);  // arrow pointing up
	static constexpr auto GC_DARROW   = static_cast<char>(25);  // arrow pointing down
	static constexpr auto GC_RARROW   = static_cast<char>(26);  // arrow pointing right
	static constexpr auto GC_LARROW   = static_cast<char>(27);  // arrow pointing left
	static constexpr auto GC_BLOCK    = static_cast<char>(219); // solid square block
	static constexpr auto GC_CKBOARD  = static_cast<char>(177); // checker board (stipple)
	static constexpr auto GC_PLMINUS  = static_cast<char>(241); // plus/minus
	static constexpr auto GC_BOARD    = static_cast<char>(176); // board of squares
	static constexpr auto GC_LRCORNER = static_cast<char>(217); // lower right corner
	static constexpr auto GC_URCORNER = static_cast<char>(191); // upper right corner
	static constexpr auto GC_ULCORNER = static_cast<char>(218); // upper left corner
	static constexpr auto GC_LLCORNER = static_cast<char>(192); // lower left corner
	static constexpr auto GC_PLUS     = static_cast<char>(197); // large plus or crossover
	static constexpr auto GC_S1       = static_cast<char>(254); // scan line 1
	static constexpr auto GC_HLINE    = static_cast<char>(196); // horizontal line
	static constexpr auto GC_LTEE     = static_cast<char>(195); // tee pointing right
	static constexpr auto GC_RTEE     = static_cast<char>(180); // tee pointing left
	static constexpr auto GC_BTEE     = static_cast<char>(193); // tee pointing up
	static constexpr auto GC_TTEE     = static_cast<char>(194); // tee pointing down
	static constexpr auto GC_VLINE    = static_cast<char>(179); // vertical line
	static constexpr auto GC_BULLET   = static_cast<char>(250); // bullet
	static constexpr auto GC_LEQUAL   = static_cast<char>(243); // less-than-or-equal-to
	static constexpr auto GC_GEQUAL   = static_cast<char>(242); // greater-than-or-equal-to
	static constexpr auto GC_DIAMOND  = static_cast<char>(4);   // diamond
	static constexpr auto GC_STERLING = static_cast<char>(156); // UK pound sign
	static constexpr auto GC_DEGREE   = static_cast<char>(248); // degree symbol
	static constexpr auto GC_PI       = static_cast<char>(227); // greek pi
};

} // namespace tui
