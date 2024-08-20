#pragma once

#include <memory>

enum class TetrominoType;

class BagOfSeven final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	BagOfSeven();
	~BagOfSeven() noexcept;

	TetrominoType next();
};
