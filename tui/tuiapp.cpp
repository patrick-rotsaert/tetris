#include "tuiapp.h"
#include "asioterminal.h"
#include "keycode.h"
#include "keymodifier.h"
#include "game.h"
#include "boardrenderer.h"
#include "inputevent.h"
#include "timer.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <spdlog/spdlog.h>

class TuiApp::impl final
{
	boost::asio::io_context ioc_;
	tui::AsioTerminal       terminal_;
	Game                    game_;

	void update()
	{
		tui::BoardRenderer::render(this->game_.board(), this->terminal_);
	}

public:
	impl()
	    : ioc_{}
	    , terminal_{ this->ioc_ }
	    , game_{ [&]() { this->update(); }, std::make_unique<tui::Timer>(this->ioc_) }
	{
		this->terminal_.cursor(false);

		this->terminal_.setKeyPressedHandler([this](int key) {
			using namespace tui;

			constexpr auto SHIFT   = static_cast<int>(KeyModifier::SHIFT);
			constexpr auto CONTROL = static_cast<int>(KeyModifier::CONTROL);
			constexpr auto ALT     = static_cast<int>(KeyModifier::ALT);
			constexpr auto META    = static_cast<int>(KeyModifier::META);

			key &= ~(SHIFT | CONTROL | ALT | META);

			switch (key)
			{
			case static_cast<int>(KeyCode::LEFT):
				return this->game_.processInputEvent(InputEvent::MOVE_LEFT);
			case static_cast<int>(KeyCode::RIGHT):
				return this->game_.processInputEvent(InputEvent::MOVE_RIGHT);
			case static_cast<int>(KeyCode::DOWN):
				return this->game_.processInputEvent(InputEvent::SOFT_DROP);
			case static_cast<int>(KeyCode::SPACE):
				return this->game_.processInputEvent(InputEvent::HARD_DROP);
			case static_cast<int>(KeyCode::UP):
			case 'X':
			case 'x':
				return this->game_.processInputEvent(InputEvent::ROTATE_CLOCKWISE);
			case 'Z':
			case 'z':
				return this->game_.processInputEvent(InputEvent::ROTATE_COUNTER_CLOCKWISE);
			case static_cast<int>(KeyCode::F1):
				return this->game_.processInputEvent(InputEvent::NEW_GAME);
			}
		});
	}

	int run()
	{
		using namespace tui;

		auto sigs = boost::asio::signal_set{ this->ioc_, SIGINT, SIGTERM };
		sigs.async_wait([&](const auto&, int sig) {
			spdlog::info("caught signal {}", sig);
			this->ioc_.stop();
		});

		this->game_.start();

		this->ioc_.run();

		return 0;
	}
};

TuiApp::TuiApp()
    : pimpl_{ std::make_unique<impl>() }
{
}

TuiApp::~TuiApp() noexcept
{
}

int TuiApp::run()
{
	return this->pimpl_->run();
}
