#pragma once

#include <QString>

class QWidget;
class QSize;
class QPoint;

class Board;

class BoardRenderer final
{
	static constexpr int PREVIEW_WIDTH  = 6;
	static constexpr int PREVIEW_HEIGHT = 4;

	QWidget* widget_;
	QString  fontFamily_{};
	int      pointSize_{};

public:
	explicit BoardRenderer(QWidget* widget);

	void recalculateFontPointSize();

	QSize size() const;

	void render(const Board& board, const QPoint& boardOrigin);
};
