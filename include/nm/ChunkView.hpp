#ifndef NM_CHUNKVIEW_HPP
#define NM_CHUNKVIEW_HPP
#include <nm/View.hpp>

namespace nm
{
	class ChunkView : public View
	{
		private:
			bool is_current_chunk(int x, int y, const Chunk& chunk) const;
			int offset_x;
			int offset_y;

			CursorData chunk_cursor {};

			bool moved = false;
		public:
			ChunkView(CursorData& cursorData, Window& main, Window& sidebar);

			void draw_main(ChunkSquareSource& squareSource, CursorMap& cursors);
			void draw_sidebar(ChunkSquareSource& squareSource, CursorMap& cursors);

			void draw_chunk(int x, int y, Chunk& chunk);
			void draw_empty_chunk(int x, int y);
			HandlerResult handle_input(int input_character);

			void switched_in_handler();
			void switched_out_handler();
	};
}

#endif
