#include "tetromino.h"
#include "tetrominocolor.h"
#include "rotationdirection.h"

#include <cassert>
#include <stdexcept>

namespace {

using TetrominoColorMapType = std::map<TetrominoType, TetrominoColor>;

const TetrominoColorMapType& tetromino_color_map()
{
	static const TetrominoColorMapType _{ { TetrominoType::J, TetrominoColor::BLUE },  { TetrominoType::L, TetrominoColor::ORANGE },
		                                  { TetrominoType::S, TetrominoColor::GREEN }, { TetrominoType::T, TetrominoColor::PURPLE },
		                                  { TetrominoType::Z, TetrominoColor::RED },   { TetrominoType::I, TetrominoColor::CYAN },
		                                  { TetrominoType::O, TetrominoColor::YELLOW } };
	return _;
}

TetrominoColor tetromino_color(TetrominoType type)
{
	auto it = tetromino_color_map().find(type);
	assert(it != tetromino_color_map().end());
	return it->second;
}

const RotationStatesMap& rotation_states_map()
{
	static const RotationStatesMap _ = { { TetrominoType::I,
		                                   RotationStates{ { { { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } } },
		                                                     { { { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 } } },
		                                                     { { { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 } } },
		                                                     { { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 } } } } } },
		                                 { TetrominoType::J,
		                                   RotationStates{ { { { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 2, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 } } },
		                                                     { { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 2, 2 } } },
		                                                     { { { 0, 2 }, { 1, 2 }, { 1, 1 }, { 1, 0 } } } } } },
		                                 { TetrominoType::L,
		                                   RotationStates{ { { { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 2, 0 } } },
		                                                     { { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 2 } } },
		                                                     { { { 0, 2 }, { 0, 1 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 } } } } } },
		                                 { TetrominoType::O,
		                                   RotationStates{ { { { { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } } } } } },
		                                 { TetrominoType::S,
		                                   RotationStates{ { { { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 2, 0 } } },
		                                                     { { { 1, 0 }, { 1, 1 }, { 2, 1 }, { 2, 2 } } },
		                                                     { { { 0, 2 }, { 1, 2 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 2 } } } } } },
		                                 { TetrominoType::T,
		                                   RotationStates{ { { { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 2, 1 } } },
		                                                     { { { 1, 0 }, { 1, 1 }, { 2, 1 }, { 1, 2 } } },
		                                                     { { { 0, 1 }, { 1, 1 }, { 1, 2 }, { 2, 1 } } },
		                                                     { { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, 2 } } } } } },
		                                 { TetrominoType::Z,
		                                   RotationStates{ { { { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 2, 1 } } },
		                                                     { { { 2, 0 }, { 2, 1 }, { 1, 1 }, { 1, 2 } } },
		                                                     { { { 0, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 } } },
		                                                     { { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 2 } } } } } } };
	return _;
}

const RotationStates& rotation_states(TetrominoType type)
{
	auto it = rotation_states_map().find(type);
	assert(it != rotation_states_map().end());
	return it->second;
}

/*
J, L, S, T, Z Tetromino Wall Kick Data
        Test 1   Test 2   Test 3   Test 4
0->R    (-1, 0)  (-1,+1)  ( 0,-2)  (-1,-2)
R->0    (+1, 0)  (+1,-1)  ( 0,+2)  (+1,+2)
R->2    (+1, 0)  (+1,-1)  ( 0,+2)  (+1,+2)
2->R    (-1, 0)  (-1,+1)  ( 0,-2)  (-1,-2)
2->L    (+1, 0)  (+1,+1)  ( 0,-2)  (+1,-2)
L->2    (-1, 0)  (-1,-1)  ( 0,+2)  (-1,+2)
L->0    (-1, 0)  (-1,-1)  ( 0,+2)  (-1,+2)
0->L    (+1, 0)  (+1,+1)  ( 0,-2)  (+1,-2)


I Tetromino Wall Kick Data
        Test 1   Test 2   Test 3   Test 4
0->R    (-2, 0)  (+1, 0)  (-2,-1)  (+1,+2)
R->0    (+2, 0)  (-1, 0)  (+2,+1)  (-1,-2)
R->2    (-1, 0)  (+2, 0)  (-1,+2)  (+2,-1)
2->R    (+1, 0)  (-2, 0)  (+1,-2)  (-2,+1)
2->L    (+2, 0)  (-1, 0)  (+2,+1)  (-1,-2)
L->2    (-2, 0)  (+1, 0)  (-2,-1)  (+1,+2)
L->0    (+1, 0)  (-2, 0)  (+1,-2)  (-2,+1)
0->L    (-1, 0)  (+2, 0)  (-1,+2)  (+2,-1)
*/

const WallKickMap& jlstz_wall_kick_map()
{
	static const WallKickMap _{ { std::make_tuple(0, RotationDirection::CLOCKWISE), // 0->R
		                          { Offset{ -1, 0 }, Offset{ -1, +1 }, Offset{ 0, -2 }, Offset{ -1, -2 } } },

		                        { std::make_tuple(1, RotationDirection::COUNTER_CLOCKWISE), // R->0
		                          { Offset{ +1, 0 }, Offset{ +1, -1 }, Offset{ 0, +2 }, Offset{ +1, +2 } } },

		                        { std::make_tuple(1, RotationDirection::CLOCKWISE), // R->2
		                          { Offset{ +1, 0 }, Offset{ +1, -1 }, Offset{ 0, +2 }, Offset{ +1, +2 } } },

		                        { std::make_tuple(2, RotationDirection::COUNTER_CLOCKWISE), // 2->R
		                          { Offset{ -1, 0 }, Offset{ -1, +1 }, Offset{ 0, -2 }, Offset{ -1, -2 } } },

		                        { std::make_tuple(2, RotationDirection::CLOCKWISE), // 2->L
		                          { Offset{ +1, 0 }, Offset{ +1, +1 }, Offset{ 0, -2 }, Offset{ +1, -2 } } },

		                        { std::make_tuple(3, RotationDirection::COUNTER_CLOCKWISE), // L->2
		                          { Offset{ -1, 0 }, Offset{ -1, -1 }, Offset{ 0, +2 }, Offset{ -1, +2 } } },

		                        { std::make_tuple(3, RotationDirection::CLOCKWISE), // L->0
		                          { Offset{ -1, 0 }, Offset{ -1, -1 }, Offset{ 0, +2 }, Offset{ -1, +2 } } },

		                        { std::make_tuple(0, RotationDirection::COUNTER_CLOCKWISE), // 0->L
		                          { Offset{ +1, 0 }, Offset{ +1, +1 }, Offset{ 0, -2 }, Offset{ +1, -2 } } } };
	return _;
}

const WallKickMap& i_wall_kick_map()
{
	static const WallKickMap _{ { std::make_tuple(0, RotationDirection::CLOCKWISE), // 0->R
		                          { Offset{ -2, 0 }, Offset{ +1, 0 }, Offset{ -2, -1 }, Offset{ +1, +2 } } },

		                        { std::make_tuple(1, RotationDirection::COUNTER_CLOCKWISE), // R->0
		                          { Offset{ +2, 0 }, Offset{ -1, 0 }, Offset{ +2, +1 }, Offset{ -1, -2 } } },

		                        { std::make_tuple(1, RotationDirection::CLOCKWISE), // R->2
		                          { Offset{ -1, 0 }, Offset{ +2, 0 }, Offset{ -1, +2 }, Offset{ +2, -1 } } },

		                        { std::make_tuple(2, RotationDirection::COUNTER_CLOCKWISE), // 2->R
		                          { Offset{ +1, 0 }, Offset{ -2, 0 }, Offset{ +1, -2 }, Offset{ -2, +1 } } },

		                        { std::make_tuple(2, RotationDirection::CLOCKWISE), // 2->L
		                          { Offset{ +2, 0 }, Offset{ -1, 0 }, Offset{ +2, +1 }, Offset{ -1, -2 } } },

		                        { std::make_tuple(3, RotationDirection::COUNTER_CLOCKWISE), // L->2
		                          { Offset{ -2, 0 }, Offset{ +1, 0 }, Offset{ -2, -1 }, Offset{ +1, +2 } } },

		                        { std::make_tuple(3, RotationDirection::CLOCKWISE), // L->0
		                          { Offset{ +1, 0 }, Offset{ -2, 0 }, Offset{ +1, -2 }, Offset{ -2, +1 } } },

		                        { std::make_tuple(0, RotationDirection::COUNTER_CLOCKWISE), // 0->L
		                          { Offset{ -1, 0 }, Offset{ +2, 0 }, Offset{ -1, +2 }, Offset{ +2, -1 } } } };
	return _;
}

std::optional<std::reference_wrapper<const WallKickMap>> wall_kick_map(TetrominoType type)
{
	switch (type)
	{
	case TetrominoType::O:
		return std::nullopt;
	case TetrominoType::I:
		return i_wall_kick_map();
	case TetrominoType::J:
	case TetrominoType::L:
	case TetrominoType::S:
	case TetrominoType::T:
	case TetrominoType::Z:
		return jlstz_wall_kick_map();
	}
	throw std::runtime_error{ "should not happen" };
}

} // namespace

class Tetromino::impl final
{
	TetrominoType                                            type_;
	TetrominoColor                                           color_;
	const RotationStates&                                    rotationStates_;
	Rotation                                                 rotation_;
	std::optional<std::reference_wrapper<const WallKickMap>> wallKickMap_;

public:
	explicit impl(TetrominoType type)
	    : type_{ type }
	    , color_{ tetromino_color(type) }
	    , rotationStates_{ rotation_states(type) }
	    , rotation_{}
	    , wallKickMap_{ wall_kick_map(type) }
	{
	}

	TetrominoType type() const
	{
		return this->type_;
	}

	TetrominoColor color() const
	{
		return this->color_;
	}

	Rotation rotation() const
	{
		return this->rotation_;
	}

	const RotationState& rotationState() const
	{
		assert(this->rotation_ >= 0 && this->rotation_ < static_cast<int>(this->rotationStates_.size()));
		return this->rotationStates_[this->rotation_];
	}

	void rotate(RotationDirection direction)
	{
		static_assert(static_cast<int>(RotationDirection::CLOCKWISE) == 1, "");
		static_assert(static_cast<int>(RotationDirection::COUNTER_CLOCKWISE) == -1, "");
		this->rotation_ = (this->rotation_ + this->rotationStates_.size() + static_cast<int>(direction)) % this->rotationStates_.size();
	}

	void rotateOpposite(RotationDirection direction)
	{
		this->rotate(static_cast<RotationDirection>(-static_cast<int>(direction)));
	}

	std::optional<std::reference_wrapper<const WallKickMap>> wallKickMap() const
	{
		return this->wallKickMap_;
	}
};

Tetromino::Tetromino(TetrominoType type)
    : pimpl_{ std::make_unique<impl>(type) }
{
}

Tetromino::~Tetromino() noexcept
{
}

TetrominoType Tetromino::type() const
{
	return this->pimpl_->type();
}

TetrominoColor Tetromino::color() const
{
	return this->pimpl_->color();
}

Rotation Tetromino::rotation() const
{
	return this->pimpl_->rotation();
}

const RotationState& Tetromino::rotationState() const
{
	return this->pimpl_->rotationState();
}

void Tetromino::rotate(RotationDirection direction)
{
	return this->pimpl_->rotate(direction);
}

void Tetromino::rotateOpposite(RotationDirection direction)
{
	return this->pimpl_->rotateOpposite(direction);
}

std::optional<std::reference_wrapper<const WallKickMap>> Tetromino::wallKickMap() const
{
	return this->pimpl_->wallKickMap();
}
