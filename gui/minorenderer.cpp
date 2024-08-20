#include "minorenderer.h"
#include "tetrominocolor.h"

#include <QPainter>

#include <cassert>

const MinoColors MinoRenderer::greyColors{ QColor{ 0x77, 0x77, 0x77 }, QColor{ 0x99, 0x99, 0x99 }, QColor{ 0x33, 0x33, 0x33 } };
const MinoColors MinoRenderer::yellowColors{ QColor{ 0xCC, 0xCC, 0x00 }, QColor{ 0xFF, 0xFF, 0x00 }, QColor{ 0x99, 0x99, 0x00 } };
const MinoColors MinoRenderer::redColors{ QColor{ 0xCC, 0x00, 0x00 }, QColor{ 0xFF, 0x00, 0x00 }, QColor{ 0x99, 0x00, 0x00 } };
const MinoColors MinoRenderer::purpleColors{ QColor{ 0x99, 0x00, 0xCC }, QColor{ 0xCC, 0x00, 0xFF }, QColor{ 0x66, 0x00, 0x99 } };
const MinoColors MinoRenderer::greenColors{ QColor{ 0x00, 0xCC, 0x00 }, QColor{ 0x00, 0xFF, 0x00 }, QColor{ 0x00, 0x99, 0x00 } };
const MinoColors MinoRenderer::orangeColors{ QColor{ 0xCC, 0x66, 0x00 }, QColor{ 0xFF, 0x88, 0x00 }, QColor{ 0x99, 0x44, 0x00 } };
const MinoColors MinoRenderer::blueColors{ QColor{ 0x00, 0x00, 0xCC }, QColor{ 0x00, 0x00, 0xFF }, QColor{ 0x00, 0x00, 0x99 } };
const MinoColors MinoRenderer::cyanColors{ QColor{ 0x00, 0xCC, 0xCC }, QColor{ 0x00, 0xFF, 0xFF }, QColor{ 0x00, 0x99, 0x99 } };

namespace {

using MinoColorsMapType = std::map<TetrominoColor, MinoColors>;

const MinoColorsMapType& mino_colors_map()
{
	static const MinoColorsMapType _{
		{ TetrominoColor::YELLOW, MinoRenderer::yellowColors }, { TetrominoColor::RED, MinoRenderer::redColors },
		{ TetrominoColor::PURPLE, MinoRenderer::purpleColors }, { TetrominoColor::GREEN, MinoRenderer::greenColors },
		{ TetrominoColor::ORANGE, MinoRenderer::orangeColors }, { TetrominoColor::BLUE, MinoRenderer::blueColors },
		{ TetrominoColor::CYAN, MinoRenderer::cyanColors }
	};
	return _;
}

const MinoColors& mino_colors(TetrominoColor color)
{
	auto it = mino_colors_map().find(color);
	assert(it != mino_colors_map().end());
	return it->second;
}

} // namespace

MinoRenderer::MinoRenderer()
{
	this->setMinoSize(DEFAULT_MINO_SIZE);
}

MinoRenderer& MinoRenderer::instance()
{
	static MinoRenderer _{};
	return _;
}

void MinoRenderer::render(QPainter& painter, const QPoint& pos, const MinoColors& minoColors)
{
	painter.setPen(Qt::PenStyle::NoPen);

	// front
	painter.setBrush(minoColors.front);
	painter.drawRect(
	    QRect{ QPoint{ this->edgeSize_, this->edgeSize_ }, QSize{ this->innerMinoSize_, this->innerMinoSize_ } }.translated(pos));

	// top
	painter.setBrush(minoColors.light);
	{
		QPolygon polygon{};
		polygon << QPoint(0, 0) << QPoint(this->minoSize_, 0) << QPoint(this->minoSize_ - this->edgeSize_, this->edgeSize_)
		        << QPoint(this->edgeSize_, this->edgeSize_);
		polygon.translate(pos);
		painter.drawPolygon(polygon);
	}

	// left
	painter.setBrush(minoColors.light);
	{
		QPolygon polygon{};
		polygon << QPoint(0, 0) << QPoint(this->edgeSize_, this->edgeSize_) << QPoint(this->edgeSize_, this->minoSize_ - this->edgeSize_)
		        << QPoint(0, this->minoSize_);
		polygon.translate(pos);
		painter.drawPolygon(polygon);
	}

	// right
	painter.setBrush(minoColors.dark);
	{
		QPolygon polygon{};
		polygon << QPoint(this->minoSize_ - this->edgeSize_, this->edgeSize_)
		        << QPoint(this->minoSize_ - this->edgeSize_, this->minoSize_ - this->edgeSize_) << QPoint(this->minoSize_, this->minoSize_)
		        << QPoint(this->minoSize_, 0);
		polygon.translate(pos);
		painter.drawPolygon(polygon);
	}

	// bottom
	painter.setBrush(minoColors.dark);
	{
		QPolygon polygon{};
		polygon << QPoint(0, this->minoSize_) << QPoint(this->edgeSize_, this->minoSize_ - this->edgeSize_)
		        << QPoint(this->minoSize_ - this->edgeSize_, this->minoSize_ - this->edgeSize_) << QPoint(this->minoSize_, this->minoSize_);
		polygon.translate(pos);
		painter.drawPolygon(polygon);
	}
}

void MinoRenderer::render(QPainter& painter, const QPoint& pos, TetrominoColor color)
{
	this->render(painter, pos, mino_colors(color));
}

int MinoRenderer::minoSize() const
{
	return this->minoSize_;
}

void MinoRenderer::setMinoSize(int minoSize)
{
	this->minoSize_      = minoSize;
	this->edgeSize_      = minoSize * EDGE_SIZE_PERCENTAGE / 100;
	this->innerMinoSize_ = minoSize - 2 * this->edgeSize_;
}
