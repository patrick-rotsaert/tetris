#pragma once

#include "itimer.h"

#include <QObject>
#include <QTimer>

#include <functional>

class Timer final : public ITimer, public QObject
{
	QTimer                timer_{};
	std::function<void()> callback_{};

	void timeout();

public:
	Timer();

	void start(int msec, std::function<void()> callback) override;
	void stop() override;
};
