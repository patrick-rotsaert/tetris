#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "asioterminal.h"
#include "asioinput.h"
#include "keycode.h"
#include "keymodifier.h"
#include "character.h"
#include "curses-config.h"

#include <boost/asio/write.hpp>
#include <boost/throw_exception.hpp>
#include <boost/system/system_error.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#if defined(HAVE_TERM_H)
#include <term.h>
#endif
#if defined(HAVE_TERMIO_H)
#include <termio.h>
#endif
#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#endif

#include <string_view>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <system_error>
#include <cctype>

#include <unistd.h>

#if defined(CURSES_HAVE_CURSES_H)
#include <curses.h>
#elif defined(CURSES_HAVE_NCURSES_H)
#include <ncurses.h>
#elif defined(CURSES_HAVE_NCURSES_NCURSES_H)
#include <ncursesw/curses.h>
#elif defined(CURSES_HAVE_NCURSES_CURSES_H)
#include <ncurses/curses.h>
#else
#error No curses header available
#endif

#include <locale.h>
#include <langinfo.h>

namespace tui {

namespace {

struct Cell
{
	Attribute::type a{};
	char            c{ ' ' };

	void clear(Attribute::type attr)
	{
		this->c = ' ';
		this->a = attr;
	}

	bool operator==(const Cell& other) const
	{
		return this->a == other.a && this->c == other.c;
	}
};

struct Line
{
	std::vector<Cell> cells{};

	Line()
	{
		this->cells.resize(columns);
	}

	std::optional<std::reference_wrapper<Cell>> cell(int x)
	{
		if (x >= 0 && x < static_cast<int>(this->cells.size()))
		{
			return std::ref(this->cells[x]);
		}
		else
		{
			return std::nullopt;
		}
	}

	void clear(Attribute::type attr)
	{
		for (auto& cell : this->cells)
		{
			cell.clear(attr);
		}
	}
};

struct Screen
{
	std::vector<Line> rows{};

	Screen()
	{
		this->rows.resize(lines);
	}

	std::optional<std::reference_wrapper<Cell>> cell(int x, int y)
	{
		if (y >= 0 && y < static_cast<int>(this->rows.size()))
		{
			return this->rows[y].cell(x);
		}
		else
		{
			return std::nullopt;
		}
	}

	void clear(Attribute::type attr)
	{
		for (auto& row : this->rows)
		{
			row.clear(attr);
		}
	}
};

class GlyphPrinter
{
	struct Glyph
	{
		char             graph;
		char             ascii;
		std::string_view utf8;
	};

	using GlyphMap = std::map<char, Glyph>;

	static const GlyphMap& map()
	{
		static GlyphMap _{
			{ Character::GC_UARROW, { '-', '^', "↑" } }, // arrow pointing up
			{ Character::GC_DARROW, { '.', 'v', "↓" } }, // arrow pointing down
			{ Character::GC_RARROW, { '+', '>', "→" } }, // arrow pointing right
			{ Character::GC_LARROW, { ',', '<', "←" } }, // arrow pointing left
			{
			    Character::GC_BLOCK,
			    { '0', '#', "█" },
			},                                             // solid square block
			{ Character::GC_CKBOARD, { 'a', ':', "░" } },  // checker board (stipple)
			{ Character::GC_PLMINUS, { 'g', '#', "±" } },  // plus/minus
			{ Character::GC_BOARD, { 'h', '#', "░" } },    // board of squares
			{ Character::GC_LRCORNER, { 'j', '+', "┘" } }, // lower right corner
			{ Character::GC_URCORNER, { 'k', '+', "┐" } }, // upper right corner
			{ Character::GC_ULCORNER, { 'l', '+', "┌" } }, // upper left corner
			{ Character::GC_LLCORNER, { 'm', '+', "└" } }, // lower left corner
			{ Character::GC_PLUS, { 'n', '+', "┼" } },     // large plus or crossover
			{ Character::GC_S1, { 'o', '-', "■" } },       // scan line 1
			{ Character::GC_HLINE, { 'q', '-', "─" } },    // horizontal line
			{ Character::GC_LTEE, { 't', '+', "├" } },     // tee pointing right
			{ Character::GC_RTEE, { 'u', '+', "┤" } },     // tee pointing left
			{ Character::GC_BTEE, { 'v', '+', "┴" } },     // tee pointing up
			{ Character::GC_TTEE, { 'w', '+', "┬" } },     // tee pointing down
			{ Character::GC_VLINE, { 'x', '|', "│" } },    // vertical line
			{ Character::GC_BULLET, { '~', 'o', "·" } },   // bullet
			{ Character::GC_LEQUAL, { 'y', '<', "≤" } },   // less-than-or-equal-to
			{ Character::GC_GEQUAL, { 'z', '>', "≥" } },   // greater-than-or-equal-to
			{ Character::GC_DIAMOND, { '`', ' ', "♦" } },  // diamond
			{ Character::GC_STERLING, { '}', 'f', "£" } }, // UK pound sign
			{ Character::GC_DEGREE, { 'f', '\\', "°" } },  // degree symbol
			{ Character::GC_PI, { '{', '*', "π" } },       // greek pi
		};
		return _;
	}

	GlyphMap map_{};
	bool     altCharsetEnabled_{};
	bool     altCharsetSelected_{};
	bool     supportsAltCharset_{};
	bool     supportsUtf8_{};

	void enableAltCharset(std::string& buffer)
	{
		if (this->supportsAltCharset_ && !this->altCharsetEnabled_)
		{
			assert(ena_acs);
			buffer.append(ena_acs);
			this->altCharsetEnabled_ = true;
		}
	}

	void enterAltCharset(std::string& buffer)
	{
		if (this->supportsAltCharset_ && !this->altCharsetSelected_)
		{
			this->enableAltCharset(buffer);
			assert(enter_alt_charset_mode);
			buffer.append(enter_alt_charset_mode);
			this->altCharsetSelected_ = true;
		}
	}

	void exitAltCharset(std::string& buffer)
	{
		if (this->supportsAltCharset_ && this->altCharsetSelected_)
		{
			this->enableAltCharset(buffer);
			assert(exit_alt_charset_mode);
			buffer.append(exit_alt_charset_mode);
			this->altCharsetSelected_ = false;
		}
	}

public:
	GlyphPrinter()
	{
		setlocale(LC_CTYPE, "");
		auto encoding = nl_langinfo(CODESET);
		if (encoding && std::string_view{ encoding } == "UTF-8")
		{
			this->supportsUtf8_ = true;
		}
	}

	void init()
	{
		this->map_ = map();
		if (acs_chars && enter_alt_charset_mode && exit_alt_charset_mode && ena_acs)
		{
			this->supportsAltCharset_ = true;
			// terminal capable of drawing glyphs
			std::string_view acsChars{ acs_chars };
			for (auto& pair : this->map_)
			{
				auto&      glyph = pair.second;
				const auto vt100 = glyph.graph;
				glyph.graph      = '\0';
				for (size_t i = 0; i < acsChars.length() / 2; ++i)
				{
					if (acsChars[2 * i] == vt100)
					{
						glyph.graph = acsChars[2 * i + 1];
						break;
					}
				}
			}
		}
		else
		{
			// terminal *not* capable of drawing glyphs
			this->supportsAltCharset_ = false;
		}
	}

	void print(std::string& buffer, char c)
	{
		if (std::isprint(c))
		{
			this->exitAltCharset(buffer);
			buffer.append(1, c);
		}
		else
		{
			auto it = this->map_.find(c);
			if (it == this->map_.end())
			{
				this->exitAltCharset(buffer);
				buffer.append(1, '?');
			}
			else
			{
				const auto& glyph = it->second;
				if (this->supportsUtf8_)
				{
					buffer.append(glyph.utf8);
				}
				else if (this->supportsAltCharset_)
				{
					if (glyph.graph)
					{
						this->enableAltCharset(buffer);
						buffer.append(1, glyph.graph);
					}
					else
					{
						this->exitAltCharset(buffer);
						buffer.append(1, glyph.ascii);
					}
				}
				else
				{
					buffer.append(1, '?');
				}
			}
		}
	}
};

class TerminalOutput final
{
	int          fd_{ STDERR_FILENO };
	int          columns_{}, rows_{};
	std::string  buffer_{};
	GlyphPrinter glyphPrinter_{};

	template<typename... Args>
	bool tparm(char* cap, Args... args)
	{
		auto s = ::tparm(cap, args...);
		if (s)
		{
			this->buffer_.append(s);
			return true;
		}
		else
		{
			return false;
		}
	}

	void flush()
	{
		if (!this->buffer_.empty())
		{
			this->write(this->buffer_);
			this->buffer_.clear();
		}
	}

	void write(std::string_view s)
	{
		auto result = ::write(this->fd_, s.data(), s.length());
		if (result < 0)
		{
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::generic_category(), "write"));
		}
	}

public:
	TerminalOutput()
	{
		// setup terminal, this causes the reading of terminfo database
		if (setupterm(0, this->fd_, 0) != OK)
		{
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::generic_category(), "setupterm"));
		}

		// test most basic capabilities
		if (!clear_screen || !cursor_address)
		{
			BOOST_THROW_EXCEPTION(std::runtime_error{ "Terminal too dumb!" });
		}

		this->rows_    = lines;
		this->columns_ = columns;
		SPDLOG_DEBUG("rows: {}, columns: {}", this->rows_, this->columns_);

		this->tparm(enter_ca_mode);

		this->attribute(Attribute::BG_BLACK);
		this->cls();

		this->flush();

		this->glyphPrinter_.init();
	}

	~TerminalOutput() noexcept
	{
		try
		{
			// turn off text attributes
			this->tparm(exit_attribute_mode);

			// set cursor visible
			this->cursor(true);

			if (!this->tparm(exit_ca_mode))
			{
				this->cls();
			}

			this->flush();
		}
		catch (const std::exception& e)
		{
			SPDLOG_WARN("{}", e.what());
		}
	}

	int fd() const
	{
		return this->fd_;
	}

	std::string unbuffer()
	{
		return std::move(this->buffer_);
	}

	Size size() const
	{
		return Size{ this->rows_, this->columns_ };
	}

	void cls(void)
	{
		this->tparm(clear_screen);
	}

	void cursor(bool on)
	{
		this->tparm(on ? cursor_normal : cursor_invisible);
	}

	void movexy(int x, int y)
	{
		if (x >= 0 && y >= 0)
		{
			this->tparm(cursor_address, y, x);
		}
	}

	void attribute(Attribute::type attr)
	{
		constexpr int fcoltab[] = { 30, 34, 32, 36, 31, 35, 33, 37 };
		constexpr int bcoltab[] = { 40, 44, 42, 46, 41, 45, 43, 47 };

		this->buffer_.append(
		    fmt::format("\x1b"
		                "[0;{};{}",
		                fcoltab[attr & 7],
		                bcoltab[(attr >> 4) & 7]));
		if (attr & Attribute::FG_BRIGHT)
		{
			this->buffer_.append(";1");
		}
		if (attr & Attribute::FG_BLINK)
		{
			this->buffer_.append(";5");
		}
		this->buffer_.append("m");
	}

	void print(std::string_view s)
	{
		for (const auto c : s)
		{
			this->glyphPrinter_.print(this->buffer_, c);
		}
	}

	void print(char c)
	{
		this->glyphPrinter_.print(this->buffer_, c);
	}
};

class TerminalInput final
{
	int     fd_{ STDIN_FILENO };
	termios tty_{};

public:
	TerminalInput()
	{
		// get terminal input attributes
		termios tty{};
		if (tcgetattr(this->fd_, &tty) < 0)
		{
			throw std::system_error(errno, std::generic_category(), "tcgetattr");
		}

		// make a copy to restore in destructor
		this->tty_ = tty;

		// Modify terminal settings for non-canonical mode and no echo
		tty.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
		tty.c_cc[VMIN]  = 1;             // Minimum number of characters to read
		tty.c_cc[VTIME] = 0;             // Timeout (0 means no timeout)

		// activate the new attributes
		if (tcsetattr(this->fd_, TCSANOW, &tty) < 0)
		{
			throw std::system_error(errno, std::generic_category(), "tcsetattr");
		}
	}

	~TerminalInput() noexcept
	{
		// restore original terminal input attributes */
		tcsetattr(this->fd_, TCSANOW, &this->tty_);
	}

	int fd() const
	{
		return this->fd_;
	}
};

} // namespace

class AsioTerminal::impl
{
	TerminalInput                         terminalInput_;
	TerminalOutput                        terminalOutput_;
	AsioInput                             input_;
	boost::asio::posix::stream_descriptor output_;
	std::unique_ptr<Screen>               lastScreen_;
	std::unique_ptr<Screen>               nextScreen_;
	Screen                                currentScreen_;
	bool                                  cursor_;
	bool                                  writeInProgress_;
	std::string                           frame_;

public:
	explicit impl(boost::asio::io_context& ioc, bool trapCtrlC, std::function<void(int)> keyPressedHandler)
	    : terminalInput_{}
	    , terminalOutput_{}
	    , input_{ ioc,
		          ::dup(terminalInput_.fd()),
		          keyPressedHandler ? keyPressedHandler : std::bind(&impl::keyPressed, this, std::placeholders::_1),
		          trapCtrlC }
	    , output_{ ioc, ::dup(terminalOutput_.fd()) }
	    , lastScreen_{}
	    , nextScreen_{}
	    , currentScreen_{}
	    , cursor_{}
	    , writeInProgress_{}
	    , frame_{}
	{
	}

	void setKeyPressedHandler(std::function<void(int key)> handler)
	{
		this->input_.setKeyPressedHandler(handler);
	}

	Size size() const
	{
		return this->terminalOutput_.size();
	}

	void cursor(bool on)
	{
		this->cursor_ = on;
	}

	void cls(Attribute::type attr)
	{
		this->currentScreen_.clear(attr);
	}

	void print(Attribute::type attr, int x, int y, std::string_view s)
	{
		for (const auto c : s)
		{
			auto optCellRef = this->currentScreen_.cell(x, y);
			if (optCellRef.has_value())
			{
				auto& cell = optCellRef.value().get();
				cell.a     = attr;
				cell.c     = c;
				++x;
			}
			else
			{
				break;
			}
		}
	}

	void print(Attribute::type attr, int x, int y, char c)
	{
		auto optCellRef = this->currentScreen_.cell(x, y);
		if (optCellRef.has_value())
		{
			auto& cell = optCellRef.value().get();
			cell.a     = attr;
			cell.c     = c;
		}
	}

	void update()
	{
		this->nextScreen_ = std::make_unique<Screen>(this->currentScreen_);
		if (!this->writeInProgress_)
		{
			this->writeNextScreen(std::move(this->nextScreen_));
		}
	}

private:
	void writeNextScreen(std::unique_ptr<Screen> screen)
	{
		this->frame_ = this->lastScreen_ ? this->renderDiff(*screen, *this->lastScreen_) : this->renderFull(*screen);
		boost::asio::async_write(this->output_,
		                         boost::asio::buffer(this->frame_),
		                         std::bind(&impl::onWrite, this, std::placeholders::_1, std::placeholders::_2));
		this->writeInProgress_ = true;
		this->lastScreen_      = std::move(screen);
	}

	void onWrite(boost::system::error_code ec, std::size_t)
	{
		this->writeInProgress_ = false;
		if (ec)
		{
			BOOST_THROW_EXCEPTION(boost::system::system_error(ec, "write"));
		}
		if (this->nextScreen_)
		{
			this->writeNextScreen(std::move(this->nextScreen_));
		}
	}

	std::string renderFull(const Screen& screen)
	{
		this->terminalOutput_.cursor(false);

		std::optional<Attribute::type> attr{};
		int                            y = 0;
		for (const auto& row : screen.rows)
		{
			this->terminalOutput_.movexy(0, y++);
			for (const auto& cell : row.cells)
			{
				if (cell.a != attr)
				{
					this->terminalOutput_.attribute(cell.a);
					attr = cell.a;
				}
				this->terminalOutput_.print(cell.c);
			}
		}

		if (this->cursor_)
		{
			this->terminalOutput_.cursor(true);
		}

		return this->terminalOutput_.unbuffer();
	}

	std::string renderDiff(const Screen& screen, const Screen& previous)
	{
		this->terminalOutput_.cursor(false);

		std::optional<Attribute::type> attr{};
		for (size_t y = 0; y < screen.rows.size(); ++y)
		{
			const auto& row    = screen.rows[y];
			auto        render = false;
			if (y < previous.rows.size())
			{
				const auto& previousRow = previous.rows[y];
				if (row.cells.size() > previousRow.cells.size())
				{
					render = true;
				}
				else
				{
					render = !std::equal(row.cells.begin(), row.cells.end(), previousRow.cells.begin());
				}
			}
			else
			{
				render = true;
			}
			if (render)
			{
				this->terminalOutput_.movexy(0, y);
				for (const auto& cell : row.cells)
				{
					if (cell.a != attr)
					{
						this->terminalOutput_.attribute(cell.a);
						attr = cell.a;
					}
					this->terminalOutput_.print(cell.c);
				}
			}
		}

		if (this->cursor_)
		{
			this->terminalOutput_.cursor(true);
		}

		return this->terminalOutput_.unbuffer();
	}

	void keyPressed(int key)
	{
		constexpr auto     SHIFT   = static_cast<int>(KeyModifier::SHIFT);
		constexpr auto     CONTROL = static_cast<int>(KeyModifier::CONTROL);
		constexpr auto     ALT     = static_cast<int>(KeyModifier::ALT);
		constexpr auto     META    = static_cast<int>(KeyModifier::META);
		fmt::memory_buffer buf{};
		if (key & SHIFT)
		{
			fmt::format_to(std::back_inserter(buf), "SHIFT ");
			key &= ~SHIFT;
		}
		if (key & CONTROL)
		{
			fmt::format_to(std::back_inserter(buf), "CONTROL ");
			key &= ~CONTROL;
		}
		if (key & ALT)
		{
			fmt::format_to(std::back_inserter(buf), "ALT ");
			key &= ~ALT;
		}
		if (key & META)
		{
			fmt::format_to(std::back_inserter(buf), "META ");
			key &= ~META;
		}
		if (std::isprint(key))
		{
			fmt::format_to(std::back_inserter(buf), "'{0:c}' 0x{0:02x}", key);
		}
		else
		{
			switch (static_cast<KeyCode>(key))
			{
			case KeyCode::BACKSPACE:
				fmt::format_to(std::back_inserter(buf), "BACKSPACE");
				break;
			case KeyCode::TAB:
				fmt::format_to(std::back_inserter(buf), "TAB");
				break;
			case KeyCode::ENTER:
				fmt::format_to(std::back_inserter(buf), "ENTER");
				break;
			case KeyCode::ESC:
				fmt::format_to(std::back_inserter(buf), "ESC");
				break;
			case KeyCode::UP:
				fmt::format_to(std::back_inserter(buf), "UP");
				break;
			case KeyCode::DOWN:
				fmt::format_to(std::back_inserter(buf), "DOWN");
				break;
			case KeyCode::RIGHT:
				fmt::format_to(std::back_inserter(buf), "RIGHT");
				break;
			case KeyCode::LEFT:
				fmt::format_to(std::back_inserter(buf), "LEFT");
				break;
			case KeyCode::INS:
				fmt::format_to(std::back_inserter(buf), "INS");
				break;
			case KeyCode::DEL:
				fmt::format_to(std::back_inserter(buf), "DEL");
				break;
			case KeyCode::PGUP:
				fmt::format_to(std::back_inserter(buf), "PGUP");
				break;
			case KeyCode::PGDOWN:
				fmt::format_to(std::back_inserter(buf), "PGDOWN");
				break;
			case KeyCode::HOME:
				fmt::format_to(std::back_inserter(buf), "HOME");
				break;
			case KeyCode::END:
				fmt::format_to(std::back_inserter(buf), "END");
				break;
			case KeyCode::F0:
				fmt::format_to(std::back_inserter(buf), "F0");
				break;
			case KeyCode::F1:
				fmt::format_to(std::back_inserter(buf), "F1");
				break;
			case KeyCode::F2:
				fmt::format_to(std::back_inserter(buf), "F2");
				break;
			case KeyCode::F3:
				fmt::format_to(std::back_inserter(buf), "F3");
				break;
			case KeyCode::F4:
				fmt::format_to(std::back_inserter(buf), "F4");
				break;
			case KeyCode::F5:
				fmt::format_to(std::back_inserter(buf), "F5");
				break;
			case KeyCode::F6:
				fmt::format_to(std::back_inserter(buf), "F6");
				break;
			case KeyCode::F7:
				fmt::format_to(std::back_inserter(buf), "F7");
				break;
			case KeyCode::F8:
				fmt::format_to(std::back_inserter(buf), "F8");
				break;
			case KeyCode::F9:
				fmt::format_to(std::back_inserter(buf), "F9");
				break;
			case KeyCode::F10:
				fmt::format_to(std::back_inserter(buf), "F10");
				break;
			case KeyCode::F11:
				fmt::format_to(std::back_inserter(buf), "F11");
				break;
			case KeyCode::F12:
				fmt::format_to(std::back_inserter(buf), "F12");
				break;
			case KeyCode::F13:
				fmt::format_to(std::back_inserter(buf), "F13");
				break;
			case KeyCode::F14:
				fmt::format_to(std::back_inserter(buf), "F14");
				break;
			case KeyCode::F15:
				fmt::format_to(std::back_inserter(buf), "F15");
				break;
			case KeyCode::F16:
				fmt::format_to(std::back_inserter(buf), "F16");
				break;
			case KeyCode::F17:
				fmt::format_to(std::back_inserter(buf), "F17");
				break;
			case KeyCode::F18:
				fmt::format_to(std::back_inserter(buf), "F18");
				break;
			case KeyCode::F19:
				fmt::format_to(std::back_inserter(buf), "F19");
				break;
			case KeyCode::F20:
				fmt::format_to(std::back_inserter(buf), "F20");
				break;
			default:
				fmt::format_to(std::back_inserter(buf), "{0:3d} 0x{0:02x}", key);
				break;
			}
		}
		SPDLOG_DEBUG("key: {}", fmt::to_string(buf));
	}
};

AsioTerminal::AsioTerminal(boost::asio::io_context& ioc, std::function<void(int)> keyPressedHandler, bool trapCtrlC)
    : pimpl_{ std::make_unique<impl>(ioc, trapCtrlC, keyPressedHandler) }
{
}

AsioTerminal::~AsioTerminal() noexcept
{
}

void AsioTerminal::setKeyPressedHandler(std::function<void(int)> keyPressedHandler)
{
	return this->pimpl_->setKeyPressedHandler(keyPressedHandler);
}

Size AsioTerminal::size() const
{
	return this->pimpl_->size();
}

void AsioTerminal::cursor(bool on)
{
	return this->pimpl_->cursor(on);
}

void AsioTerminal::cls(Attribute::type attr)
{
	return this->pimpl_->cls(attr);
}

void AsioTerminal::print(Attribute::type attr, int x, int y, std::string_view s)
{
	return this->pimpl_->print(attr, x, y, s);
}

void AsioTerminal::print(Attribute::type attr, int x, int y, char c)
{
	return this->pimpl_->print(attr, x, y, c);
}

void AsioTerminal::print(Attribute::type attr, const Position& pos, std::string_view s)
{
	return this->pimpl_->print(attr, pos.x, pos.y, s);
}

void AsioTerminal::print(Attribute::type attr, const Position& pos, char c)
{
	return this->pimpl_->print(attr, pos.x, pos.y, c);
}

void AsioTerminal::update()
{
	return this->pimpl_->update();
}

} // namespace tui
