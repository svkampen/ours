#include <nm/BoardView.hpp>
#include <nm/Typedefs.hpp>
#include <ncurses.h>
#include <boost/log/trivial.hpp>
#include <nm/Utils.hpp>

namespace nm
{
	BoardView::BoardView(CursorData& cursorData, Window& main, Window& sidebar, const SquareEvent& open, const SquareEvent& flag, const SquareEvent& move)
		: ev_square_open(open), ev_square_flag(flag), ev_cursor_move(move), View(cursorData, main, sidebar)
	{};


	void BoardView::draw_main(ChunkSquareSource& squareSource, std::unordered_map<int32_t, CursorData>& others) {
		int XViewable = main.cols / 3;
		int YViewable = main.lines;

		main << nm::Erase;

		for (int y = 0; y < YViewable; y++)
		{
			for (int x = 0; x < XViewable; x++)
			{
				main << Move({3 * x + 1, y});
				auto& square = squareSource.get({x + cursor.offset_x, y + cursor.offset_y});
				if (square.is_mine && square.state == SquareState::OPENED)
				{
					main << L"*";
				} else if (square.state == SquareState::OPENED)
				{
					this->draw_open_square(x, y, square);
				} else if (square.state == SquareState::FLAGGED)
				{
					this->draw_flag_square(x, y, square);
				} else
				{
					this->draw_closed_square(x, y, square);
				}
			}
		}


		this->draw_cursors(others);
		this->draw_cursor(squareSource, others);

		main << Refresh;
	}

	void BoardView::draw_cursor(ChunkSquareSource& css, CursorMap& cm)
	{
		main << Move({3 * cursor.x, cursor.y}) << "["
			<< Move({3 * cursor.x + 2, cursor.y}) << "]";

		main << Refresh;
	}

	void BoardView::center_cursor()
	{

		center_cursor(cursor.x + cursor.offset_x, cursor.y + cursor.offset_y);
	}

	void BoardView::center_cursor(int global_x, int global_y)
	{
		cursor.offset_x = global_x - main.cols / 6;
		cursor.x = main.cols / 6;

		cursor.offset_y = global_y - main.lines / 2;
		cursor.y = main.lines / 2;

		this->ev_cursor_move(global_x, global_y);
	}


	HandlerResult BoardView::handle_input(int input_character)
	{
		bool full_redraw = false;
		switch(input_character)
		{
			case ' ':
				this->ev_square_open(cursor.offset_x + cursor.x, cursor.offset_y + cursor.y);
				full_redraw = true;
				break;
			case 'c':
				this->center_cursor();
				full_redraw = true;
				break;
			case '0':
				this->center_cursor(0, 0);
				full_redraw = true;
				break;
			case 'b':
				this->border_enabled = !this->border_enabled;
				full_redraw = true;
				break;
			case 'f':
				this->ev_square_flag(cursor.offset_x + cursor.x, cursor.offset_y + cursor.y);
				full_redraw = true;
				break;

			/* ARROW KEYS */
			case KEY_LEFT:
				if (cursor.x > 0)
				{
					cursor.x--;
				}
				else
				{
					cursor.offset_x -= 1;
					full_redraw = true;
				}
				break;
			case KEY_RIGHT:
				if (cursor.x < (main.cols / 3) - 1)
				{
					cursor.x++;
				}
				else
				{
					cursor.offset_x += 1;
					full_redraw = true;
				}
				break;
			case KEY_UP:
				if (cursor.y > 0)
				{
					cursor.y--;
				}
				else
				{
					cursor.offset_y -= 1;
					full_redraw = true;
				}
				break;
			case KEY_DOWN:
				if (cursor.y < main.lines - 1)
				{
					cursor.y++;
				}
				else
				{
					cursor.offset_y += 1;
					full_redraw = true;
				}
				break;
			default:
				break;
		}

		switch (input_character)
		{
			case KEY_LEFT:
			case KEY_RIGHT:
			case KEY_UP:
			case KEY_DOWN:
				this->ev_cursor_move(cursor.x + cursor.offset_x, cursor.y + cursor.offset_y);
				break;
			default:
				break;
		}

		if (full_redraw)
			return HandlerResult::DRAW_ALL;

		return HandlerResult::DRAW_CURSORS;
	}

	void BoardView::draw_sidebar(ChunkSquareSource& squareSource, std::unordered_map<int32_t, CursorData>& cursors)
	{
		sidebar << nm::Erase
			<< L"   Netamphetamine   "
			<< L"        Help        "
			<< L"┌──────────────────┐"
			<< L"│ V to open chunk  │"
			<< L"│      view.       │"
			<< L"│                  │"
			<< L"│ F to flag.       │"
			<< L"│ Q to quit.       │"
			<< L"│ C to center.     │"
			<< L"│ B to show chunks.│"
			<< L"│ 0 to goto origin.│"
			<< L"│ SPC to open sqre.│"
			<< L"└──────────────────┘\n";

		sidebar
			<< L"       Clients      "
			<< L"┌──────────────────┐";

		for (auto&& pair : cursors)
		{
			sidebar << L"│ "
				<< nm::AttrOn(COLOR_PAIR(pair.second.color)) << L"o " << nm::AttrOff(COLOR_PAIR(pair.second.color))
				<< utils::int_to_hex(pair.first) << L"       │";

			sidebar << L"│   (" << pair.second.x << L", " << pair.second.y << ")";

			int line = sidebar.gety();

			sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});
		}

		sidebar
			<< L"└──────────────────┘\n";

		sidebar
			<< L"      Infostats     "
			<< L"┌──────────────────┐"
			<< L"│ Chn: " << this->chunk_x() << ", " << this->chunk_y();

		int line = sidebar.gety();
		sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});

		sidebar
			<< L"│ Pos: " << this->global_x() << ", " << this->global_y();

		line = sidebar.gety();
		sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});

		sidebar
			<< L"└──────────────────┘\n";


		sidebar << nm::Refresh;
	}

	void BoardView::draw_flag_square(int x, int y, Square& square)
	{
		int global_x = x + cursor.offset_x;
		int global_y = y + cursor.offset_y;

		bool chunk_border = (global_x % NM_CHUNK_SIZE == 0 || global_y % NM_CHUNK_SIZE == 0) && this->border_enabled;

		if (chunk_border)
			main << AttrOn(COLOR_PAIR(BORDER_COLOR));

		main << Move({3 * x, y}) << " # ";

		if (chunk_border)
			main << AttrOff(COLOR_PAIR(BORDER_COLOR));
	}

	void BoardView::draw_closed_square(int x, int y, Square& square)
	{
		main << Move({3 * x, y}) << AttrOn(COLOR_PAIR(7)) << "   " << AttrOff(COLOR_PAIR(7));
	}

	void BoardView::draw_cursors(std::unordered_map<int32_t, CursorData>& cursors)
	{
		int width, height;
		getyx((WINDOW*)main, height, width);

		for (auto iterator = cursors.cbegin(); iterator != cursors.cend(); iterator++)
		{
			int x = iterator->second.x - cursor.offset_x;
			int y = iterator->second.y - cursor.offset_y;

			if (x < 0 || y < 0)
				continue;

			if (x >= width || y >= height)
				continue;

			wattron(main, COLOR_PAIR(iterator->second.color));

			if (wmove(main, y, 3*x) != ERR)
				waddwstr(main, L"[");
			if (wmove(main, y, 3*x+2) != ERR)
				waddwstr(main, L"]");

			wattroff(main, COLOR_PAIR(iterator->second.color));
		}
	}

	void BoardView::draw_open_square(int x, int y, Square& square)
	{
		int color = 0;
		switch(square.number)
		{
			case 0:
				color = -1;
				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				color = square.number + 1;
				break;
			case 6:
			case 7:
				color = 6;
				break;
			case 8:
				color = 5;
				break;
		}

		int global_x = x + cursor.offset_x;
		int global_y = y + cursor.offset_y;

		bool chunk_border = (global_x % NM_CHUNK_SIZE == 0 || global_y % NM_CHUNK_SIZE == 0) && this->border_enabled;

		if (color == -1)
		{
			if (chunk_border)
				main << AttrOn(COLOR_PAIR(BORDER_COLOR));

			main << Move({3 * x, y}) << L"   ";

			if (chunk_border)
				main << AttrOff(COLOR_PAIR(BORDER_COLOR));

			return;
		}

		if (chunk_border)
			color = BORDER_COLOR;

		main << AttrOn(COLOR_PAIR(color)) << nm::Move({3 * x, y})
			 << ' ' << static_cast<char>(square.number + 48) << ' ' << AttrOff(COLOR_PAIR(color));
	}
}
