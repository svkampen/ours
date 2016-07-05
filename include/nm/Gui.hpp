#ifndef _NM_GUI_
#define _NM_GUI_

#include <ncurses.h>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <nm/Game.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Flag.hpp>

namespace nm
{
	class Gui : public boost::enable_shared_from_this<Gui>
	{
		private:
			WINDOW* main_window;
			int width;
			int height;
			int board_offset_x, board_offset_y, cursor_x, cursor_y;

			Flag resized;

		public:
			typedef boost::signals2::signal<void (int, int)> square_event;
			square_event ev_square_open;
			square_event ev_square_flag;
			square_event ev_cursor_move;
			boost::signals2::signal<void ()> ev_exit;

			Gui();
			void handle_input();
			void draw_open_square(Square& square);
			void draw_board(SquareSource& squareSource);
			static void handle_resize();
			void poll(SquareSource& squareSource);
	};
}

#endif //_NM_GUI_
