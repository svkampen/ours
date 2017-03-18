#include <nm/Gui.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Flag.hpp>
#include <nm/Utils.hpp>

#include <sys/ioctl.h>
#include <signal.h>

#include <boost/log/trivial.hpp>

namespace nm
{
	void init_curses()
	{
		setlocale(LC_ALL, "");
		initscr();
		srand(time(NULL));

		if (has_colors())
		{
			start_color();
			use_default_colors();
			init_pair(1, COLOR_WHITE,	-1);
			init_pair(2, COLOR_BLUE,	-1);
			init_pair(3, COLOR_CYAN,	-1);
			init_pair(4, COLOR_GREEN,	-1);
			init_pair(5, COLOR_MAGENTA, -1);
			init_pair(6, COLOR_RED,		-1);

			// Inverted colors

			init_pair(7, COLOR_BLACK,	COLOR_WHITE);
			init_pair(8, COLOR_BLUE,	COLOR_WHITE);
			init_pair(9, COLOR_CYAN,	COLOR_WHITE);
			init_pair(10,COLOR_GREEN,	COLOR_WHITE);
			init_pair(11,COLOR_MAGENTA,	COLOR_WHITE);
			init_pair(12,COLOR_RED,		COLOR_WHITE);

			// Red backgrounded

			init_pair(13,COLOR_BLACK,	COLOR_RED);
			init_pair(14,COLOR_BLUE,	COLOR_RED);
			init_pair(15,COLOR_CYAN,	COLOR_RED);
			init_pair(16,COLOR_GREEN,	COLOR_RED);
			init_pair(17,COLOR_MAGENTA,	COLOR_RED);
			init_pair(18,COLOR_WHITE,	COLOR_RED);
		}

		noecho();
		cbreak();
		keypad(stdscr, TRUE);
		nodelay(stdscr, TRUE);
		curs_set(0);
	}



	Gui::Gui(boost::asio::io_service& io_service, SquareSource& squareSource)
		: main(0, 0, COLS - 21, LINES), sidebar(COLS - 20, 0, 20, LINES),
			squareSource(squareSource),	in(io_service, ::dup(STDIN_FILENO)),
			boardview(self_cursor, main, ev_square_open, ev_square_flag, ev_cursor_move), current_view(boardview)
	{
		width = height = 0;
		handle_resize();

		center_cursor(0, 0);

		in.async_read_some(boost::asio::null_buffers(), boost::bind(&Gui::draw, this));
	}

	void Gui::player_quit_handler(const message::Player& player)
	{
		cursors.erase(player.id());
		draw_board();
		draw_sidebar();
	}

	void Gui::new_player_handler(const message::Player& player)
	{
		BOOST_LOG_TRIVIAL(info) << "Adding cursor for new player with ID " << player.id();
		cursors[player.id()] = {player.x(), player.y(), rand() % 6 + 2};
		draw_board();
		draw_sidebar();
	}

	void Gui::cursor_move_handler(const message::CursorMove& msg)
	{
		BOOST_LOG_TRIVIAL(info) << "Moving cursor for " << msg.id();
		auto &&data = cursors[msg.id()];
		data.x = msg.x();
		data.y = msg.y();
		draw_board();
		draw_sidebar();
	}

	void Gui::handle_resize()
	{
		struct winsize w;

		ioctl(0, TIOCGWINSZ, &w);
		COLS = w.ws_col;
		LINES = w.ws_row;

		clear();

		resize_term(LINES, COLS);

		this->width = COLS - 21;
		this->height = LINES;

		this->main.resize(0, 0, this->width, this->height);
		this->sidebar.resize(COLS - 20, 0, 20, LINES);

		this->center_cursor();

		refresh();

		draw_sidebar();
	}

	void Gui::draw()
	{
		handle_input();
		draw_board();
		draw_sidebar();
		in.async_read_some(boost::asio::null_buffers(), boost::bind(&Gui::draw, this));
	}

	inline int Gui::global_x()
	{
		return this->self_cursor.x + this->self_cursor.offset_x;
	}

	inline int Gui::global_y()
	{
		return this->self_cursor.y + this->self_cursor.offset_y;
	}

	inline int Gui::chunk_x()
	{
		return std::floor(global_x() / (double)NM_CHUNK_SIZE);
	}

	inline int Gui::chunk_y()
	{
		return std::floor(global_y() / (double)NM_CHUNK_SIZE);
	}

	void Gui::draw_sidebar()
	{
		this->sidebar << nm::Erase
			<< L"   NETAMPHETAMINE   "
			<< L"        HELP        "
			<< L"┌──────────────────┐"
			<< L"│     F - Flag     │"
			<< L"│     Q - Quit     │"
			<< L"│     C - Center   │"
			<< L"│  B - Show border │"
			<< L"│ 0 - Go to (0, 0) │"
			<< L"│ Space - Open SQR │"
			<< L"└──────────────────┘\n";

		this->sidebar
			<< L"┌──────────────────┐"
			<< L"│      CLIENTS     │"
			<< L"│                  │";

		for (auto&& pair : this->cursors)
		{
			this->sidebar << L"│ "
				<< nm::AttrOn(COLOR_PAIR(pair.second.color)) << L"\u26AB " << nm::AttrOff(COLOR_PAIR(pair.second.color))
				<< utils::int_to_hex(pair.first) << L"       │";

			this->sidebar << L"│   (" << pair.second.x << L", " << pair.second.y << ")";
			int line = this->sidebar.gety();

			this->sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});
		}

		this->sidebar
			<< L"└──────────────────┘\n";

		this->sidebar
			<< L"┌──────────────────┐"
			<< L"│     INFOSTATS    │"
			<< L"│                  │"
			<< L"│ Chn: " << this->chunk_x() << ", " << this->chunk_y();

		int line = this->sidebar.gety();
		this->sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});

		this->sidebar
			<< L"│ Pos: " << this->global_x() << ", " << this->global_y();

		line = this->sidebar.gety();
		this->sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});

		this->sidebar
			<< L"└──────────────────┘\n";


		this->sidebar << nm::Refresh;
	}

	void Gui::center_cursor()
	{

		center_cursor(this->self_cursor.x + this->self_cursor.offset_x, this->self_cursor.y + this->self_cursor.offset_y);
	}

	void Gui::center_cursor(int global_x, int global_y)
	{
		this->self_cursor.offset_x = global_x - this->width / 6;
		this->self_cursor.x = this->width / 6;

		this->self_cursor.offset_y = global_y - this->height / 2;
		this->self_cursor.y = this->height / 2;

		this->ev_cursor_move(global_x, global_y);
	}

	void Gui::save_png()
	{
		this->ev_save_image();
	}

	bool Gui::handle_input()
	{
		int ch;

		// Use this in the redraw later; if just the movement has been
		// handled, don't redraw the whole board - just the cursors
		while ((ch = getch()) != ERR)
		{
			switch(ch)
			{
				case 'f':
					this->ev_square_flag(self_cursor.offset_x + self_cursor.x, self_cursor.offset_y + self_cursor.y);
					break;

				case 'q':
					this->ev_exit();
					break;

				case ' ':
					this->ev_square_open(self_cursor.offset_x + self_cursor.x, self_cursor.offset_y + self_cursor.y);
					break;

				case '0':
					this->center_cursor(0, 0);
					break;

				case 'b':
					this->border_enabled = !this->border_enabled;
					break;

				case 'c':
					this->center_cursor();
					break;

				case 'p':
					this->save_png();
					break;


				/* ARROW KEYS */
				case KEY_LEFT:
					if (self_cursor.x > 0)
					{
						self_cursor.x--;
					}
					else
					{
						self_cursor.offset_x -= 2;
					}
					break;

				case KEY_RIGHT:
					if (self_cursor.x < (this->width / 3) - 1)
					{
						self_cursor.x++;
					}
					else
					{
						self_cursor.offset_x += 2;
					}
					break;

				case KEY_UP:
					if (self_cursor.y > 0)
					{
						self_cursor.y--;
					}
					else
					{
						self_cursor.offset_y -= 2;
					}
					break;

				case KEY_DOWN:
					if (self_cursor.y < this->height - 1)
					{
						self_cursor.y++;
					}
					else
					{
						self_cursor.offset_y += 2;
					}
					break;

				case KEY_RESIZE:
					handle_resize();
					break;
			}

			switch (ch)
			{
				case KEY_LEFT:
				case KEY_RIGHT:
				case KEY_UP:
				case KEY_DOWN:
					this->ev_cursor_move(self_cursor.x + self_cursor.offset_x, self_cursor.y + self_cursor.offset_y);
					break;
				default:
					break;
			}
		}

		return false;
	}

	void Gui::draw_board()
	{
		this->current_view.draw_main(this->squareSource, this->cursors);
	}

	Square& Gui::get(int x, int y) const
	{
		return squareSource.get({x + self_cursor.offset_x, y + self_cursor.offset_y});
	}
}
