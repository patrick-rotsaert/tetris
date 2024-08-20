#pragma once

#include <memory>

class Tetromino;
class GridPosition;
class Grid;
struct Offset;
enum class RotationDirection;

class PlayingTetromino final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit PlayingTetromino(std::unique_ptr<Tetromino> tetromino, GridPosition&& position, Grid& grid);
	~PlayingTetromino() noexcept;

	const Tetromino&    tetromino() const;
	const GridPosition& position() const;

	bool rotate(RotationDirection direction);
	bool move(const Offset& offs);
	int  hardDrop();
	bool canDescend();
};
