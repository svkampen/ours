#include <nm/BoardView.hpp>
#include <nm/Typedefs.hpp>
#include <ncurses.h>
#include <boost/log/trivial.hpp>

namespace nm
{
	BoardView::BoardView(CursorData& cursorData, Window& main, const SquareEvent& open, const SquareEvent& flag, const SquareEvent& move)
		: ev_square_open(open), ev_square_flag(flag), ev_cursor_move(move), main(main), View(cursorData)
	{};


	void BoardView::draw_main(SquareSource& squareSource, std::unordered_map<int32_t, CursorData>& others) {
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
					this->draw_open_square(main, x, y, square);
				} else if (square.state == SquareState::FLAGGED)
				{
					this->draw_flag_square(main, x, y, square);
				} else
				{
					this->draw_closed_square(main, x, y, square);
				}
			}
		}

		this->draw_cursors(main, others);

		main << Move({3 * cursor.x, cursor.y}) << "["
			<< Move({3 * cursor.x + 2, cursor.y}) << "]";

		main << Refresh;
	}

	{
	void BoardView::draw_flag_square(Window& main, int x, int y, Square& square)
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

	void BoardView::draw_closed_square(Window& main, int x, int y, Square& square)
	{
		main << Move({3 * x, y}) << AttrOn(COLOR_PAIR(7)) << "   " << AttrOff(COLOR_PAIR(7));
	}

	void BoardView::draw_cursors(Window& main, std::unordered_map<int32_t, CursorData>& cursors)
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

	void BoardView::draw_open_square(Window& main, int x, int y, Square& square)
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
