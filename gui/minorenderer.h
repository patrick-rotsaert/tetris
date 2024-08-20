#pragma once

#include "minocolors.h"

class QPainter;
class QPoint;

enum class TetrominoColor;

class MinoRenderer final
{
	static constexpr int DEFAULT_MINO_SIZE    = 20;
	static constexpr int EDGE_SIZE_PERCENTAGE = 10;

	int minoSize_{};
	int innerMinoSize_{};
	int edgeSize_{};

	MinoRenderer();

public:
	static const MinoColors greyColors;
	static const MinoColors yellowColors;
	static const MinoColors redColors;
	static const MinoColors purpleColors;
	static const MinoColors greenColors;
	static const MinoColors orangeColors;
	static const MinoColors blueColors;
	static const MinoColors cyanColors;

	static MinoRenderer& instance();

	void render(QPainter& painter, const QPoint& pos, const MinoColors& minoColors);
	void render(QPainter& painter, const QPoint& pos, TetrominoColor color);

	int minoSize() const;

	void setMinoSize(int minoSize);
};
