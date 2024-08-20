#pragma once

#include "offset.h"
#include "tetrominotype.h"

#include <memory>
#include <optional>
#include <tuple>
#include <map>
#include <functional>

enum class TetrominoColor;
enum class RotationDirection;

using Rotation          = int;
using RotationState     = std::array<Offset, 4>;
using RotationStates    = std::array<RotationState, 4>;
using RotationStatesMap = std::map<TetrominoType, RotationStates>;

using WallKickKey = std::tuple<Rotation, RotationDirection>;
using WallKicks   = std::array<Offset, 4>;
using WallKickMap = std::map<WallKickKey, WallKicks>;

class Tetromino final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit Tetromino(TetrominoType type);
	~Tetromino() noexcept;

	TetrominoType        type() const;
	TetrominoColor       color() const;
	Rotation             rotation() const;
	const RotationState& rotationState() const;

	void rotate(RotationDirection direction);
	void rotateOpposite(RotationDirection direction);

	std::optional<std::reference_wrapper<const WallKickMap>> wallKickMap() const;
};
