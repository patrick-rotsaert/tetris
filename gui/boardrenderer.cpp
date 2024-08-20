#include "boardrenderer.h"
#include "minorenderer.h"
#include "grid.h"
#include "board.h"
#include "gridposition.h"
#include "tetromino.h"

#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QWidget>

BoardRenderer::BoardRenderer(QWidget* widget)
    : widget_{ widget }
    , fontFamily_{ QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/fonts/tetris-mania-type.ttf")).at(0) }
{
	this->recalculateFontPointSize();
}

void BoardRenderer::recalculateFontPointSize()
{
	const auto targetHeight = MinoRenderer::instance().minoSize();

	QFont font{ this->fontFamily_ };

	for (this->pointSize_ = 1;; ++this->pointSize_)
	{
		font.setPointSize(this->pointSize_);

		QFontMetrics fm{ font };
		const auto   height = fm.height();
		if (height > targetHeight)
		{
			--this->pointSize_;
			break;
		}
		else if (height == targetHeight)
		{
			break;
		}
	}

	//qDebug() << "pointSize =" << this->pointSize_;
}

QSize BoardRenderer::size() const
{
	const auto minoSize = MinoRenderer::instance().minoSize();
	return QSize{
		(1 + Grid::width() + 1 + 1 + 1 + PREVIEW_WIDTH + 1) *
		    minoSize, // border left + grid columns + border right + space + border left + preview columns + border right
		(1 + Grid::height() + 1) * minoSize // border top + grid rows + border bottom
	};
}

/*
 ############ ########
 #          # #      #
 #          # # @@@@ #
 #          # #      #
 #          # #      #
 #          # ########
 #          #
 #          # Level
 #          # 1
 #          #
 #          # Lines
 #          # 15
 #          #
 #          # Score
 #          # 123450
 #    @     #
 #    @     #
 #    @@    #
 #          #
 # @@       #
 #@@@@  @@@ #
 ############
 */
void BoardRenderer::render(const Board& board, const QPoint& boardOrigin)
{
	QPainter painter{ this->widget_ };

	auto& minoRenderer = MinoRenderer::instance();

	const auto minoSize = minoRenderer.minoSize();
	auto       origin   = boardOrigin;

	// vertical borders
	for (int row = 0; row < Grid::height() + 2; ++row)
	{
		// Left
		minoRenderer.render(painter, QPoint{ 0, row * minoSize } + origin, MinoRenderer::greyColors);
		// Right
		minoRenderer.render(painter, QPoint{ (1 + Grid::width()) * minoSize, row * minoSize } + origin, MinoRenderer::greyColors);
	}

	// horizontal borders
	for (int column = 0; column < Grid::width(); ++column)
	{
		// Top
		minoRenderer.render(painter, QPoint{ (1 + column) * minoSize, 0 } + origin, MinoRenderer::greyColors);
		// Bottom
		minoRenderer.render(painter, QPoint{ (1 + column) * minoSize, (1 + Grid::height()) * minoSize } + origin, MinoRenderer::greyColors);
	}

	// grid
	// shift the origin one mino down and right (i.e inside the border).
	origin += QPoint{ minoSize, minoSize };
	for (int row = 0; row < Grid::height(); ++row)
	{
		for (int column = 0; column < Grid::width(); ++column)
		{
			const auto& cell = board.grid().cell(row, column);
			if (cell.has_value())
			{
				minoRenderer.render(painter, QPoint{ column * minoSize, row * minoSize } + origin, cell.value());
			}
		}
	}

	// Preview border
	origin = boardOrigin + QPoint{ (1 + Grid::width() + 1 + 1) * minoSize, 0 };
	// vertical borders
	for (int row = 0; row < PREVIEW_HEIGHT + 2; ++row)
	{
		// Left
		minoRenderer.render(painter, QPoint{ 0, row * minoSize } + origin, MinoRenderer::greyColors);
		// Right
		minoRenderer.render(painter, QPoint{ (1 + PREVIEW_WIDTH) * minoSize, row * minoSize } + origin, MinoRenderer::greyColors);
	}

	// horizontal borders
	for (int column = 0; column < PREVIEW_WIDTH; ++column)
	{
		// Top
		minoRenderer.render(painter, QPoint{ (1 + column) * minoSize, 0 } + origin, MinoRenderer::greyColors);
		// Bottom
		minoRenderer.render(painter, QPoint{ (1 + column) * minoSize, (1 + PREVIEW_HEIGHT) * minoSize } + origin, MinoRenderer::greyColors);
	}

	const auto& nextTetromino = board.nextTetromino();
	// shift the origin one mino down and right (i.e inside the border).
	origin += QPoint{ minoSize, minoSize };
	const auto position = GridPosition{ 1, 1 };
	for (const auto& offs : nextTetromino.rotationState())
	{
		const auto row    = position.row + offs.y;
		const auto column = position.column + offs.x;
		minoRenderer.render(painter, QPoint{ column * minoSize, row * minoSize } + origin, nextTetromino.color());
	}

	painter.setFont(QFont{ this->fontFamily_, this->pointSize_ });
	painter.setRenderHint(QPainter::Antialiasing);

	const auto& levelColors = MinoRenderer::yellowColors;
	const auto& linesColors = MinoRenderer::greenColors;
	const auto& scoreColors = MinoRenderer::cyanColors;

	// Level
	origin = boardOrigin + QPoint{ (1 + Grid::width() + 1 + 1) * minoSize, (1 + PREVIEW_HEIGHT + 1 + 1 + 2) * minoSize };
	painter.setPen(levelColors.front);
	painter.drawText(origin, "Level");

	origin += QPoint{ 0, minoSize };
	painter.setPen(levelColors.light);
	painter.drawText(origin, QString{ "%1" }.arg(board.level()));

	// Lines
	origin += QPoint{ 0, minoSize * 2 };
	painter.setPen(linesColors.front);
	painter.drawText(origin, "Lines");

	origin += QPoint{ 0, minoSize };
	painter.setPen(linesColors.light);
	painter.drawText(origin, QString{ "%1" }.arg(board.lines()));

	// Score
	origin += QPoint{ 0, minoSize * 2 };
	painter.setPen(scoreColors.front);
	painter.drawText(origin, "Score");

	origin += QPoint{ 0, minoSize };
	painter.setPen(scoreColors.light);
	painter.drawText(origin, QString{ "%1" }.arg(board.score()));

	// GAME OVER
	if (board.gameOver())
	{
		painter.setFont(QFont{ this->fontFamily_, this->pointSize_ * 2 });
		painter.setPen(MinoRenderer::redColors.light);
		origin += QPoint{ 0, minoSize * 4 };
		painter.drawText(origin, "Game");
		origin += QPoint{ 0, minoSize * 2 };
		painter.drawText(origin, "Over");
	}
}
