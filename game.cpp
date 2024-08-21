#include "game.h"
#include "board.h"
#include "grid.h"
#include "offset.h"
#include "bagofseven.h"
#include "rotationdirection.h"
#include "inputevent.h"
#include "itimer.h"
#include "playingtetromino.h"

#include <QDebug>

#include <stdexcept>

class Game::impl final
{
	static constexpr uint32_t LINES_LEVEL_UP  = 10;
	static constexpr int      LOCKING_DELAY   = 500;
	static constexpr int      SOFT_DROP_SCORE = 1;
	static constexpr int      HARD_DROP_SCORE = 2;

	std::function<void()>   onUpdate_;
	std::unique_ptr<ITimer> timer_;
	BagOfSeven              bagOfSeven_{};
	std::unique_ptr<Board>  board_{};
	uint32_t                linesForLevelUp_{ LINES_LEVEL_UP };
	bool                    lockScheduled_{};

	void reset()
	{
		this->board_ = std::make_unique<Board>(this->bagOfSeven_.next());
		this->board_->moveNextTetrominoToGrid(this->bagOfSeven_.next());
		this->onUpdate_();
		this->scheduleDescent();
	}

	int descendInterval() const
	{
		const auto level = this->board_->level();
		if (level >= 19)
		{
			return 1000 * 4 / 60;
		}
		else if (level >= 9)
		{
			return 1000 * 6 / 60;
		}
		else
		{
			return 1000 * (48 - 5 * level) / 60;
		}
	}

	void scheduleDescent()
	{
		this->timer_->start(this->descendInterval(), std::bind(&impl::descend, this));
		this->lockScheduled_ = false;
		//qDebug() << "descent scheduled";
	}

	void scheduleLock()
	{
		if (!this->lockScheduled_)
		{
			this->timer_->start(LOCKING_DELAY, std::bind(&impl::lock, this));
			this->lockScheduled_ = true;
			//qDebug() << "lock scheduled";
		}
	}

	void descend()
	{
		auto playingTetromino = this->board_->playingTetromino();
		if (playingTetromino)
		{
			const auto changed = playingTetromino->move(Offset{ 0, +1 });
			if (changed)
			{
				//qDebug() << "descended";
				this->afterChange();
			}
		}
	}

	void lock()
	{
		//qDebug() << "lock";

		this->clearFullLines();

		if (!this->board_->moveNextTetrominoToGrid(this->bagOfSeven_.next()))
		{
			return gameOver();
		}

		this->afterChange();
	}

	void afterChange()
	{
		//qDebug() << "after change";
		auto playingTetromino = this->board_->playingTetromino();
		if (playingTetromino)
		{
			if (playingTetromino->canDescend())
			{
				//qDebug() << "can descend";
				this->scheduleDescent();
			}
			else
			{
				//qDebug() << "cannot descend";
				this->scheduleLock();
			}

			this->onUpdate_();
		}
	}

	void clearFullLines()
	{
		const auto linesCleared = this->board_->grid().clearFullLines();
		if (linesCleared)
		{
			int score{};
			switch (linesCleared)
			{
			case 1:
				score = 40;
				break;
			case 2:
				score = 100;
				break;
			case 3:
				score = 300;
				break;
			case 4:
				score = 1200;
				break;
			default:
				throw std::runtime_error{ "should not happen" };
			}
			score *= (this->board_->level() + 1);
			this->board_->addScore(score);
		}

		const auto totalLinesCleared = this->board_->lines() + linesCleared;
		this->board_->setLines(totalLinesCleared);

		while (totalLinesCleared >= this->linesForLevelUp_)
		{
			this->levelUp();
			this->linesForLevelUp_ += LINES_LEVEL_UP;
		}
	}

	void levelUp()
	{
		this->board_->setLevel(this->board_->level() + 1);
	}

	void gameOver()
	{
		qDebug() << "GAME OVER";
		this->board_->setGameOver();
		this->timer_->stop();
		this->onUpdate_();
	}

	void newGame()
	{
		if (this->board_->gameOver())
		{
			this->reset();
		}
	}

public:
	explicit impl(std::function<void()> onUpdate, std::unique_ptr<ITimer> timer)
	    : onUpdate_{ onUpdate }
	    , timer_{ std::move(timer) }
	{
		this->reset();
	}

	Board& board()
	{
		return *this->board_;
	}

	void processInputEvent(InputEvent event)
	{
		auto playingTetromino = this->board_->playingTetromino();
		if (playingTetromino)
		{
			auto changed = false;

			switch (event)
			{
			case InputEvent::MOVE_LEFT:
				changed = playingTetromino->move(Offset{ -1, 0 });
				break;
			case InputEvent::MOVE_RIGHT:
				changed = playingTetromino->move(Offset{ +1, 0 });
				break;
			case InputEvent::SOFT_DROP:
				changed = playingTetromino->move(Offset{ 0, +1 });
				if (changed)
				{
					this->board_->addScore(SOFT_DROP_SCORE);
				}
				break;
			case InputEvent::HARD_DROP:
			{
				const auto rowsDropped = playingTetromino->hardDrop();
				changed                = rowsDropped > 0;
				if (changed)
				{
					this->board_->addScore(HARD_DROP_SCORE * rowsDropped);
					return this->lock();
				}
				break;
			}
			case InputEvent::ROTATE_CLOCKWISE:
				changed = playingTetromino->rotate(RotationDirection::CLOCKWISE);
				break;
			case InputEvent::ROTATE_COUNTER_CLOCKWISE:
				changed = playingTetromino->rotate(RotationDirection::COUNTER_CLOCKWISE);
				break;
			case InputEvent::NEW_GAME:
				return newGame();
			}

			if (changed)
			{
				this->afterChange();
			}
		}
	}
};

Game::Game(std::function<void()> onUpdate, std::unique_ptr<ITimer> timer)
    : pimpl_{ std::make_unique<impl>(onUpdate, std::move(timer)) }
{
}

Game::~Game() noexcept
{
}

Board& Game::board()
{
	return this->pimpl_->board();
}

void Game::processInputEvent(InputEvent event)
{
	return this->pimpl_->processInputEvent(event);
}
