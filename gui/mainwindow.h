#pragma once

#include <QWidget>

#include <memory>

class Game;

namespace gui {

class BoardRenderer;

}

class MainWindow final : public QWidget
{
	Q_OBJECT

	QMargins                            margins_;
	std::unique_ptr<gui::BoardRenderer> boardRenderer_{};
	std::unique_ptr<Game>               game_{};

	void paintEvent(QPaintEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
};
