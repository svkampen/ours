#ifndef _NM_VIEW_
#define _NM_VIEW_

#include <ncurses.h>
#include <netmine.pb.h>
#include <nm/SquareSource.hpp>
#include <nm/Chunk.hpp>
#include <nm/Window.hpp>
#include <nm/CursorData.hpp>
#include <unordered_map>

namespace nm
{
	enum HandlerResult
	{
		STOP,
		CONTINUE
	};

	class View
	{
		public:
			View(CursorData& cursorData) : cursor(cursorData) {};
			CursorData& cursor;

			virtual void draw_main(SquareSource& squareSource,
					std::unordered_map<int32_t, CursorData>& others) = 0;
			virtual void draw_sidebar(Window& sidebar, SquareSource& squareSource,
					std::unordered_map<int32_t, CursorData>& cursors) {};
			virtual HandlerResult handle_input(int input_character) { return HandlerResult::CONTINUE; };
			virtual ~View()
			{
			};


			virtual void player_quit_handler(const message::Player& player)
			{
			};

			virtual void player_join_handler(const message::Player& player)
			{
			};

			virtual void cursor_move_handler(const message::CursorMove& msg)
			{
			};
	};
}

#endif //_NM_VIEW_
