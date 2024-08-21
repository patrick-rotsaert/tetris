#include "gui/mainwindow.h"

#include <QApplication>
#include <QDebug>

#if defined(Q_OS_MACOS)
#include <CoreGraphics/CGSession.h>
#endif

#include <cstdlib>

namespace {

int gui(int argc, char* argv[])
{
	QApplication a{ argc, argv };
	MainWindow   w{};
	w.show();
	return a.exec();
}

int tui(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	qWarning() << "No display, TUI not yet implemented";
	return 1;
}

} // namespace

int main(int argc, char* argv[])
{
#if defined(Q_OS_WIN)
	// On Windows, assume a graphical display is always available
	return gui(argc, argv);
#elif defined(Q_OS_MACOS)
	// On macOS, check if we're in a graphical session
	const auto sessionDict = CGSessionCopyCurrentDictionary();
	const auto isGraphical = sessionDict != nullptr;
	if (sessionDict)
	{
		CFRelease(sessionDict);
	}
	if (isGraphical)
	{
		return gui(argc, argv);
	}
	else
	{
		return tui(argc, argv);
	}
#elif defined(Q_OS_UNIX)
	// On Unix-like systems (e.g., Linux), check if DISPLAY or WAYLAND_DISPLAY is set
	if (std::getenv("DISPLAY") != nullptr || std::getenv("WAYLAND_DISPLAY") != nullptr)
	{
		return gui(argc, argv);
	}
	else
	{
		return tui(argc, argv);
	}
#else
	// For other platforms, default to TUI
	return tui(argc, argv);
#endif
}
