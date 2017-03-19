#include <nm/View.hpp>

namespace nm
{
	class ChunkView : public View
	{
		public:
			ChunkView(CursorData& cursorData, Window& main, Window& sidebar);

			void draw_main(SquareSource& squareSource, CursorMap& cursors);
			void draw_sidebar(SquareSource& squareSource, CursorMap& cursors);
	};
}

