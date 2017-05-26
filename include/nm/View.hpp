#ifndef _NM_VIEW_
#define _NM_VIEW_

#include <ncurses.h>
#include <netmine.pb.h>
#include <nm/ChunkSquareSource.hpp>
#include <nm/Chunk.hpp>
#include <nm/Window.hpp>
#include <unordered_map>
#include <cmath>

namespace nm
{
	enum HandlerResult
	{
		DRAW_ALL,
		DRAW_CURSORS
	};

	class View
	{
		public:
			View(CursorData& cursorData, Window& main, Window& sidebar)
				: cursor(cursorData), sidebar(sidebar), main(main) {};
			CursorData& cursor;
			Window& sidebar;
			Window& main;

			virtual void draw_main(ChunkSquareSource& squareSource, CursorMap& others) = 0;
			virtual void draw_sidebar(ChunkSquareSource& squareSource, CursorMap& cursors) {};
			virtual void draw_cursor(ChunkSquareSource& squareSource, CursorMap& others) {};
			virtual HandlerResult handle_input(int input_character) { return HandlerResult::DRAW_ALL; };

			virtual void center_cursor() {}
			virtual void center_cursor(int x, int y) {}

			virtual void switched_in_handler() {}
			virtual void switched_out_handler() {}

			inline int global_x()
			{
				return cursor.x + cursor.offset_x;
			}

			inline int global_y()
			{
				return cursor.y + cursor.offset_y;
			}

			inline int chunk_x()
			{
				return std::floor(global_x() / (double)NM_CHUNK_SIZE);
			}

			inline int chunk_y()
			{
				return std::floor(global_y() / (double)NM_CHUNK_SIZE);
			}


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
