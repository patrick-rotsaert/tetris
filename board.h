#pragma once

#include <memory>
#include <cstdint>

class Grid;
class PlayingTetromino;
class Tetromino;
enum class TetrominoType;

class Board final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit Board(TetrominoType nextType);
	~Board() noexcept;

	Grid&       grid();
	const Grid& grid() const;

	PlayingTetromino*       playingTetromino();
	const PlayingTetromino* playingTetromino() const;

	Tetromino&       nextTetromino();
	const Tetromino& nextTetromino() const;

	bool moveNextTetrominoToGrid(TetrominoType nextType);

	uint32_t level() const;
	uint32_t lines() const;
	uint64_t score() const;
	bool     gameOver() const;

	void setLevel(uint32_t level);
	void setLines(uint32_t lines);
	void setScore(uint64_t score);
	void addScore(uint64_t score);
	void setGameOver();
};
