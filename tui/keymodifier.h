#pragma once

namespace tui {

enum class KeyModifier : int
{
	SHIFT   = 0x1000,
	ALT     = 0x2000,
	CONTROL = 0x4000,
	META    = 0x8000
};

}
