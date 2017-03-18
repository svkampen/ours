#ifndef _NM_BOARD_VIEW_
#define _NM_BOARD_VIEW_

#include <nm/View.hpp>

namespace nm
{
	class BoardView : public View
	{
		public:

		BoardView(CursorData& cursorData, Window& main, const SquareEvent& open, const SquareEvent& flag, const SquareEvent& move);

		const SquareEvent& ev_square_open;
		const SquareEvent& ev_square_flag;
		const SquareEvent& ev_cursor_move;

		Window& main;

		bool border_enabled = false;

		void draw_flag_square(Window& main, int x, int y,
				Square& square);

		void draw_open_square(Window& main, int x, int y,
				Square& square);

		void draw_closed_square(Window& main, int x, int y,
				Square& square);

		void draw_cursors(Window& main, std::unordered_map<int32_t, CursorData>& cursors);

		void draw_sidebar(Window& sidebar, SquareSource& squareSource, std::unordered_map<int32_t, CursorData>& cursors);
		void draw_main(SquareSource& squareSource,
				std::unordered_map<int32_t, CursorData>& others);

		HandlerResult handle_input(int input_character);
	};
}

#endif //_NM_BOARD_VIEW_
