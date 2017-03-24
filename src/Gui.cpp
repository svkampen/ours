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

			init_pair(19, COLOR_BLACK, COLOR_GREEN);
		}

		noecho();
		cbreak();
		keypad(stdscr, TRUE);
		nodelay(stdscr, TRUE);
		curs_set(0);
	}



	Gui::Gui(boost::asio::io_service& io_service, ChunkSquareSource& squareSource)
		: main(0, 0, COLS - 21, LINES), sidebar(COLS - 20, 0, 20, LINES),
			squareSource(squareSource),	in(io_service, ::dup(STDIN_FILENO)),
			boardview(self_cursor, main, sidebar, ev_square_open, ev_square_flag, ev_cursor_move), chunkview(self_cursor, main, sidebar),
			current_view(&boardview)
	{
		width = height = 0;
		handle_resize();

		this->current_view->center_cursor(0, 0);

		in.async_read_some(boost::asio::null_buffers(), boost::bind(&Gui::draw, this));
	}

	void Gui::player_quit_handler(const message::Player& player)
	{
		cursors.erase(player.id());
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void Gui::new_player_handler(const message::Player& player)
	{
		BOOST_LOG_TRIVIAL(info) << "Adding cursor for new player with ID " << player.id();
		cursors[player.id()] = {.x = player.x(), .y = player.y(), .color = rand() % 4 + 2};
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void Gui::cursor_move_handler(const message::CursorMove& msg)
	{
		BOOST_LOG_TRIVIAL(info) << "Moving cursor for " << msg.id();
		auto &&data = cursors[msg.id()];
		data.x = msg.x();
		data.y = msg.y();
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
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

		this->current_view->center_cursor();

		refresh();

		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void Gui::draw()
	{
		handle_input();
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
		in.async_read_some(boost::asio::null_buffers(), boost::bind(&Gui::draw, this));
	}

	void Gui::save_png()
	{
		this->ev_save_image();
	}

	void Gui::switch_views()
	{
		this->current_view->switched_out_handler();
		if (this->chunk_view_enabled)
		{
			this->current_view = &this->boardview;
		} else
		{
			this->current_view = &this->chunkview;
		}

		this->current_view->switched_in_handler();
		this->chunk_view_enabled = !this->chunk_view_enabled;
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
				case 'v':
					this->switch_views();
					break;

				case 'q':
					this->ev_exit();
					break;

				case 'p':
					this->save_png();
					break;

				case KEY_RESIZE:
					handle_resize();
					break;
			}

			this->current_view->handle_input(ch);
		}

		return false;
	}

	void Gui::draw_board()
	{
		this->current_view->draw_main(this->squareSource, this->cursors);
	}
}
