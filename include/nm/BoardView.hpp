#ifndef _NM_BOARD_VIEW_
#define _NM_BOARD_VIEW_

#include <nm/View.hpp>

namespace nm
{
	class BoardView : public View
	{
		bool border_enabled = false;

		void draw_flag_square(Window& main, int x, int y,
				Square& square, CursorData& cdata);

		void draw_open_square(Window& main, int x, int y,
				Square& square, CursorData &cdata);

		void draw_closed_square(Window& main, int x, int y,
				Square& square, CursorData &cdata);

		void draw_cursors(Window& main, std::unordered_map<int32_t, CursorData>& cursors,
				CursorData& cdata);

		void draw_main(Window& main, SquareSource& squareSource,
				std::unordered_map<int32_t, CursorData> others, CursorData& self);

	};
}

#endif //_NM_BOARD_VIEW_
