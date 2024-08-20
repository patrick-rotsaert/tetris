#include "board.h"
#include "tetrominotype.h"
#include "gridposition.h"
#include "grid.h"
#include "tetromino.h"
#include "playingtetromino.h"
#include <cassert>

class Board::impl final
{
	Grid                              grid_{};
	std::unique_ptr<PlayingTetromino> playingTetromino_{};
	std::unique_ptr<Tetromino>        nextTetromino_{};
	uint32_t                          level_{ 0 };
	uint32_t                          lines_{ 0 };
	uint64_t                          score_{ 0 };
	bool                              gameOver_{};

	static constexpr GridPosition initialTetrominoPosition()
	{
		return GridPosition{ 1, (Grid::width() - 4) / 2 };
	}

	void setNextTetromino(TetrominoType type)
	{
		this->nextTetromino_ = std::make_unique<Tetromino>(type);
	}

public:
	explicit impl(TetrominoType nextType)
	{
		this->setNextTetromino(nextType);
	}

	Grid& grid()
	{
		return this->grid_;
	}

	const Grid& grid() const
	{
		return this->grid_;
	}

	PlayingTetromino* playingTetromino()
	{
		return this->playingTetromino_.get();
	}

	const PlayingTetromino* playingTetromino() const
	{
		return this->playingTetromino_.get();
	}

	Tetromino& nextTetromino()
	{
		return *this->nextTetromino_.get();
	}

	const Tetromino& nextTetromino() const
	{
		return *this->nextTetromino_.get();
	}

	bool moveNextTetrominoToGrid(TetrominoType nextType)
	{
		assert(this->nextTetromino_);
		auto position = initialTetrominoPosition();
		if (this->grid_.accepts(*this->nextTetromino_, position))
		{
			this->playingTetromino_ = std::make_unique<PlayingTetromino>(std::move(this->nextTetromino_), std::move(position), this->grid_);
			this->setNextTetromino(nextType);
			return true;
		}
		else
		{
			return false;
		}
	}

	uint32_t level() const
	{
		return this->level_;
	}

	uint32_t lines() const
	{
		return this->lines_;
	}

	uint64_t score() const
	{
		return this->score_;
	}

	void setLevel(uint32_t level)
	{
		this->level_ = level;
	}

	void setLines(uint32_t lines)
	{
		this->lines_ = lines;
	}

	void setScore(uint64_t score)
	{
		this->score_ = score;
	}

	void addScore(uint64_t score)
	{
		this->score_ += score;
	}

	void setGameOver()
	{
		this->gameOver_ = true;
		this->playingTetromino_.reset();
	}

	bool gameOver() const
	{
		return this->gameOver_;
	}
};

Board::Board(TetrominoType nextType)
    : pimpl_{ std::make_unique<impl>(nextType) }
{
}

Board::~Board() noexcept
{
}

Grid& Board::grid()
{
	return this->pimpl_->grid();
}

const Grid& Board::grid() const
{
	return this->pimpl_->grid();
}

PlayingTetromino* Board::playingTetromino()
{
	return this->pimpl_->playingTetromino();
}

const PlayingTetromino* Board::playingTetromino() const
{
	return this->pimpl_->playingTetromino();
}

Tetromino& Board::nextTetromino()
{
	return this->pimpl_->nextTetromino();
}

const Tetromino& Board::nextTetromino() const
{
	return this->pimpl_->nextTetromino();
}

bool Board::moveNextTetrominoToGrid(TetrominoType nextType)
{
	return this->pimpl_->moveNextTetrominoToGrid(nextType);
}

uint32_t Board::level() const
{
	return this->pimpl_->level();
}

uint32_t Board::lines() const
{
	return this->pimpl_->lines();
}

uint64_t Board::score() const
{
	return this->pimpl_->score();
}

bool Board::gameOver() const
{
	return this->pimpl_->gameOver();
}

void Board::setLevel(uint32_t level)
{
	return this->pimpl_->setLevel(level);
}

void Board::setLines(uint32_t lines)
{
	return this->pimpl_->setLines(lines);
}

void Board::setScore(uint64_t score)
{
	return this->pimpl_->setScore(score);
}

void Board::addScore(uint64_t score)
{
	return this->pimpl_->addScore(score);
}

void Board::setGameOver()
{
	return this->pimpl_->setGameOver();
}
