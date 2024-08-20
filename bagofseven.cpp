#include "bagofseven.h"
#include "tetrominotype.h"

#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

class BagOfSeven::impl final
{
	std::vector<TetrominoType> bag_{};
	std::random_device         rd_{};

	void fill()
	{
		assert(this->bag_.empty());
		this->bag_.push_back(TetrominoType::J);
		this->bag_.push_back(TetrominoType::L);
		this->bag_.push_back(TetrominoType::S);
		this->bag_.push_back(TetrominoType::T);
		this->bag_.push_back(TetrominoType::Z);
		this->bag_.push_back(TetrominoType::I);
		this->bag_.push_back(TetrominoType::O);
		std::mt19937 g{ this->rd_() };
		std::shuffle(this->bag_.begin(), this->bag_.end(), g);
	}

public:
	impl()
	{
		this->fill();
	}

	TetrominoType next()
	{
		if (this->bag_.empty())
		{
			this->fill();
		}
		const auto result = this->bag_.back();
		this->bag_.pop_back();
		return result;
	}
};

BagOfSeven::BagOfSeven()
    : pimpl_{ std::make_unique<impl>() }
{
}

BagOfSeven::~BagOfSeven() noexcept
{
}

TetrominoType BagOfSeven::next()
{
	return this->pimpl_->next();
}
