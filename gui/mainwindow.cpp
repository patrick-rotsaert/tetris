#include "mainwindow.h"
#include "game.h"
#include "boardrenderer.h"
#include "inputevent.h"
#include "timer.h"

#include <QKeyEvent>

void MainWindow::paintEvent(QPaintEvent*)
{
	this->boardRenderer_->render(this->game_->board(), QPoint{ this->margins_.left(), this->margins_.top() });
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Left:
		return this->game_->processInputEvent(InputEvent::MOVE_LEFT);
	case Qt::Key_Right:
		return this->game_->processInputEvent(InputEvent::MOVE_RIGHT);
	case Qt::Key_Down:
		return this->game_->processInputEvent(InputEvent::SOFT_DROP);
	case Qt::Key_Space:
		return this->game_->processInputEvent(InputEvent::HARD_DROP);
	case Qt::Key_Up:
	case Qt::Key_X:
		return this->game_->processInputEvent(InputEvent::ROTATE_CLOCKWISE);
	case Qt::Key_Control:
	case Qt::Key_Z:
		return this->game_->processInputEvent(InputEvent::ROTATE_COUNTER_CLOCKWISE);
	case Qt::Key_F1:
		return this->game_->processInputEvent(InputEvent::NEW_GAME);
	}
}

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , margins_{ 20, 20, 20, 20 }
    , boardRenderer_{ std::make_unique<gui::BoardRenderer>(this) }
    , game_{ std::make_unique<Game>([this]() { this->update(); }, std::make_unique<gui::Timer>()) }
{
	auto palette = this->palette();
	palette.setColor(QPalette::Window, Qt::black);
	this->setPalette(palette);

	this->setAutoFillBackground(true);

	this->setFixedSize(this->boardRenderer_->size().grownBy(this->margins_));

	this->game_->start();
}

MainWindow::~MainWindow()
{
}
