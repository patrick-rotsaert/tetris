#include "gui/mainwindow.h"
#include "tui/tuiapp.h"

#include <QApplication>
#include <QDebug>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#if defined(Q_OS_MACOS)
#include <CoreGraphics/CGSession.h>
#endif

#include <cstdlib>
#include <iostream>

namespace {

int run_gui(int argc, char* argv[])
{
	QApplication a{ argc, argv };
	MainWindow   w{};
	w.show();
	return a.exec();
}

int run_tui(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		return TuiApp{}.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
}

} // namespace

int main(int argc, char* argv[])
{
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("tetris.log", false);
	sink->set_level(spdlog::level::trace);

	auto logger = std::make_shared<spdlog::logger>("", sink);
	logger->flush_on(spdlog::level::trace);

	spdlog::set_default_logger(logger);
	spdlog::set_level(spdlog::level::trace);

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
		return run_gui(argc, argv);
	}
	else
	{
		return run_tui(argc, argv);
	}
#elif defined(Q_OS_UNIX)
	// On Unix-like systems (e.g., Linux), check if DISPLAY or WAYLAND_DISPLAY is set
	if (std::getenv("DISPLAY") != nullptr || std::getenv("WAYLAND_DISPLAY") != nullptr)
	{
		return run_gui(argc, argv);
	}
	else
	{
		return run_tui(argc, argv);
	}
#else
	// For other platforms, default to TUI
	return run_tui(argc, argv);
#endif
}
