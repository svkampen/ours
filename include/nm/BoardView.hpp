#ifndef _NM_BOARD_VIEW_
#define _NM_BOARD_VIEW_

#include <nm/View.hpp>

namespace nm
{
	class BoardView : public View
	{
		public:

		BoardView(CursorData& cursorData, Window& main, Window& sidebar, const SquareEvent& open, const SquareEvent& flag, const SquareEvent& move);

		const SquareEvent& ev_square_open;
		const SquareEvent& ev_square_flag;
		const SquareEvent& ev_cursor_move;

		bool border_enabled = false;

		void draw_flag_square(int x, int y,
				Square& square);

		void draw_open_square(int x, int y,
				Square& square);

		void draw_closed_square(int x, int y,
				Square& square);

		void draw_cursors(CursorMap& cursors);

		void draw_sidebar(SquareSource& squareSource, CursorMap& cursors);

		void draw_main(SquareSource& squareSource, CursorMap& others);
		HandlerResult handle_input(int input_character);

		private:
		void center_cursor();
		void center_cursor(int global_x, int global_y);
	};
}

#endif //_NM_BOARD_VIEW_
