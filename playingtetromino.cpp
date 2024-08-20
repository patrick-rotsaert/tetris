#include "playingtetromino.h"
#include "tetromino.h"
#include "gridposition.h"
#include "grid.h"

#include <cassert>

class PlayingTetromino::impl final
{
	std::unique_ptr<Tetromino> tetromino_;
	GridPosition               position_;
	Grid&                      grid_;

	void addToGrid()
	{
		for (const auto& offs : this->tetromino_->rotationState())
		{
			const auto row    = this->position_.row + offs.y;
			const auto column = this->position_.column + offs.x;
			assert(!this->grid_.cell(row, column).has_value());
			this->grid_.cell(row, column) = this->tetromino_->color();
		}
	}

	void removeFromGrid()
	{
		for (const auto& offs : this->tetromino_->rotationState())
		{
			const auto row    = this->position_.row + offs.y;
			const auto column = this->position_.column + offs.x;
			assert(this->grid_.cell(row, column).has_value());
			this->grid_.cell(row, column) = std::nullopt;
		}
	}

	std::optional<GridPosition> tryRotation(RotationDirection direction)
	{
		const auto currentRotation = this->tetromino_->rotation();

		this->tetromino_->rotate(direction);

		if (this->grid_.accepts(*this->tetromino_, this->position_))
		{
			return this->position_;
		}

		if (this->tetromino_->wallKickMap())
		{
			const auto& wallKickMap = this->tetromino_->wallKickMap().value().get();
			const auto  wallKickKey = std::make_tuple(currentRotation, direction);
			const auto  it          = wallKickMap.find(wallKickKey);
			assert(it != wallKickMap.end());
			const auto& wallKicks = it->second;
			for (const auto& wallKick : wallKicks)
			{
				const auto position = GridPosition{ this->position_.row - wallKick.y, this->position_.column + wallKick.x };
				if (this->grid_.accepts(*this->tetromino_, position))
				{
					return position;
				}
			}
		}

		return std::nullopt;
	}

public:
	explicit impl(std::unique_ptr<Tetromino> tetromino, GridPosition&& position, Grid& grid)
	    : tetromino_{ std::move(tetromino) }
	    , position_{ std::move(position) }
	    , grid_{ grid }
	{
		this->addToGrid();
	}

	const Tetromino& tetromino() const
	{
		return *this->tetromino_;
	}

	const GridPosition& position() const
	{
		return this->position_;
	}

	bool rotate(RotationDirection direction)
	{
		this->removeFromGrid();

		const auto position = this->tryRotation(direction);

		if (position)
		{
			this->position_ = position.value();
			this->addToGrid();
			return true;
		}
		else
		{
			this->tetromino_->rotateOpposite(direction);
			this->addToGrid();
			return false;
		}
	}

	bool move(const Offset& offs)
	{
		this->removeFromGrid();

		const auto position = GridPosition{ this->position_.row + offs.y, this->position_.column + offs.x };
		if (this->grid_.accepts(*this->tetromino_, position))
		{
			this->position_ = position;
			this->addToGrid();
			return true;
		}
		else
		{
			this->addToGrid();
			return false;
		}
	}

	int hardDrop()
	{
		this->removeFromGrid();

		int rowsDropped{};

		for (;;)
		{
			auto position = this->position_;
			++position.row;

			if (this->grid_.accepts(*this->tetromino_, position))
			{
				this->position_ = position;
				++rowsDropped;
			}
			else
			{
				break;
			}
		}

		this->addToGrid();

		return rowsDropped;
	}

	bool canDescend()
	{
		this->removeFromGrid();

		auto position = this->position_;
		++position.row;

		const auto result = this->grid_.accepts(*this->tetromino_, position);

		this->addToGrid();
		return result;
	}
};

PlayingTetromino::PlayingTetromino(std::unique_ptr<Tetromino> tetromino, GridPosition&& position, Grid& grid)
    : pimpl_{ std::make_unique<impl>(std::move(tetromino), std::move(position), grid) }
{
}

PlayingTetromino::~PlayingTetromino() noexcept
{
}

const Tetromino& PlayingTetromino::tetromino() const
{
	return this->pimpl_->tetromino();
}

const GridPosition& PlayingTetromino::position() const
{
	return this->pimpl_->position();
}

bool PlayingTetromino::rotate(RotationDirection direction)
{
	return this->pimpl_->rotate(direction);
}

bool PlayingTetromino::move(const Offset& offs)
{
	return this->pimpl_->move(offs);
}

int PlayingTetromino::hardDrop()
{
	return this->pimpl_->hardDrop();
}

bool PlayingTetromino::canDescend()
{
	return this->pimpl_->canDescend();
}
