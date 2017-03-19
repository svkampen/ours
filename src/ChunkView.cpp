#include <nm/ChunkView.hpp>

namespace nm
{
	ChunkView::ChunkView(CursorData& cursorData, Window& main, Window& sidebar)
		: View(cursorData, main, sidebar) {};

	void ChunkView::draw_main(SquareSource& squareSource, CursorMap& cursors) {
		main << nm::Erase;

		int XViewable = main.cols / 3;
		int YViewable = main.lines;

		for (int y = 0; y < YViewable; y++)
		{
			for (int x = 0; x < XViewable; x++)
			{
				main << Move({3 * x + 1, y});

			}
		}
	}

	void ChunkView::draw_sidebar(SquareSource& squareSource, CursorMap& cursors) {

	}
}
