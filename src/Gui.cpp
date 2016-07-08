#include <nm/Gui.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Flag.hpp>

#include <sys/ioctl.h>
#include <signal.h>

namespace nm
{
	Gui::Gui(boost::asio::io_service& io_service, SquareSource& squareSource) : squareSource(squareSource),
		in(io_service, ::dup(STDIN_FILENO))
	{
		width = height = board_offset_x = board_offset_y = 0;

		initscr();

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
			init_pair(7, COLOR_BLACK,	COLOR_WHITE);
			init_pair(8, COLOR_BLUE,	COLOR_WHITE);
			init_pair(9, COLOR_CYAN,	COLOR_WHITE);
			init_pair(10,COLOR_GREEN,	COLOR_WHITE);
			init_pair(11,COLOR_MAGENTA,	COLOR_WHITE);
			init_pair(12,COLOR_RED,		COLOR_WHITE);
		}

		noecho();
		cbreak();
		keypad(stdscr, TRUE);
		nodelay(stdscr, TRUE);

		this->main_window = stdscr;

		handle_resize();

		this->cursor_x = (COLS / 2)/3;
		this->cursor_y = LINES / 2;

		Gui::draw();
		in.async_read_some(boost::asio::null_buffers(), boost::bind(&Gui::draw, this));
	}

	}

	void Gui::handle_resize()
	{
		struct winsize w;

		ioctl(0, TIOCGWINSZ, &w);
		COLS = w.ws_col;
		LINES = w.ws_row;

		clear();

		resize_term(LINES, COLS);
		wresize(stdscr, LINES, COLS);
		wresize(curscr, LINES, COLS);

		refresh();
	}

	void Gui::draw()
	{
		handle_input();
		draw_board();
		in.async_read_some(boost::asio::null_buffers(), boost::bind(&Gui::draw, this));
	}

	void Gui::handle_input()
	{
		int ch;
		if ((ch = getch()) == ERR)
			return;

		switch(ch)
		{
			case 'f':
				this->ev_square_flag(this->board_offset_x + this->cursor_x, this->board_offset_y + this->cursor_y);
				break;

			case 'q':
				this->ev_exit();
				break;

			case ' ':
				this->ev_square_open(this->board_offset_x + this->cursor_x, this->board_offset_y + this->cursor_y);
				break;


			/* ARROW KEYS */
			case KEY_LEFT:
				if (cursor_x > 0)
					cursor_x--;
				else
					board_offset_x -= 10;
				break;

			case KEY_RIGHT:
				if (cursor_x < this->width)
					cursor_x++;
				else
					board_offset_x += 10;
				break;

			case KEY_UP:
				if (cursor_y > 0)
					cursor_y--;
				else
					board_offset_y -= 10;
				break;

			case KEY_DOWN:
				if (cursor_y < this->height - 1)
					cursor_y++;
				else
					board_offset_y += 10;
				break;

			case KEY_RESIZE:
				handle_resize();
				break;
		}
	}

	void Gui::draw_board()
	{
		this->width = COLS;
		this->height = LINES;

		int XViewable = this->width/3;
		int YViewable = this->height;

		werase(this->main_window);

		for (int y = 0; y < YViewable; y++)
		{
			for (int x = 0; x < XViewable; x++)
			{
				wmove(this->main_window, y, 3*x+1);
				auto& square = squareSource.get({x + this->board_offset_x, y + board_offset_y});
				if (square.is_mine && square.state == SquareState::OPENED)
				{
					waddch(this->main_window, '*');
				} else if (square.state == SquareState::OPENED) 
				{
					this->draw_open_square(square);
				} else if (square.state == SquareState::FLAGGED)
				{
					waddch(this->main_window, '#');
				} else
				{
					waddch(this->main_window, '-');
				}
			}
		}

		wmove(this->main_window, cursor_y, 3*cursor_x+1);

		wrefresh(this->main_window);
	}

	void Gui::draw_open_square(Square& square)
	{
		switch(square.number)
		{
			case 0:
				waddch(this->main_window, ' '); break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				wcolor_set(this->main_window, square.number + 1, NULL);
				waddch(this->main_window, static_cast<char>(square.number + 48));
				wcolor_set(this->main_window, 1, NULL);
				break;
			case 6:
			case 7:
				wcolor_set(this->main_window, 6, NULL);
				waddch(this->main_window, static_cast<char>(square.number + 48));
				wcolor_set(this->main_window, 1, NULL);
				break;
			case 8:
				wcolor_set(this->main_window, 5, NULL);
				waddch(this->main_window, static_cast<char>(square.number + 48));
				wcolor_set(this->main_window, 1, NULL);
				break;
		}
	}
}
