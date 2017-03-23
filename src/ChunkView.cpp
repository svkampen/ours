#include <nm/ChunkView.hpp>
#include <nm/Utils.hpp>

namespace nm
{
	ChunkView::ChunkView(CursorData& cursorData, Window& main, Window& sidebar)
		: View(cursorData, main, sidebar) {};

	void ChunkView::draw_main(ChunkSquareSource& squareSource, CursorMap& cursors) {
		main << nm::Erase;

		int XViewable = main.cols / 3;
		int YViewable = main.lines;

		offset_x = -XViewable / 2;
		offset_y = -YViewable / 2;

		for (int y = 0; y < YViewable; y++)
		{
			for (int x = 0; x < XViewable; x++)
			{
				main << Move({3*x+1, y});
				auto opt_chunk = squareSource.get_chunk({x + offset_x, y + offset_y});
				if (opt_chunk.is_initialized())
				{
					this->draw_chunk(x, y, opt_chunk.get());
				} else
				{
					this->draw_empty_chunk(x, y);
				}

			}
		}

		main << nm::Refresh;
	}

	bool ChunkView::is_current_chunk(int x, int y, const Chunk& chunk) const
	{
		int global_chunk_x = x + offset_x;
		int global_chunk_y = y + offset_y;

		int global_cursor_x = cursor.x + cursor.offset_x;
		int global_cursor_y = cursor.y + cursor.offset_y;

		auto c = utils::to_chunk_coordinates({global_cursor_x, global_cursor_y});

		return (global_chunk_x == c.x() && global_chunk_y == c.y());
	}

	void ChunkView::draw_chunk(int x, int y, Chunk& chunk)
	{
		main << Move({3 * x, y});
		auto to_write = this->is_current_chunk(x, y, chunk) ? L"[ ]" : L"   ";

		if (chunk.all_squares([](const auto& square) {
				return square.state == SquareState::OPENED || square.state == SquareState::FLAGGED;
			}))
		{
			main << AttrOn(COLOR_PAIR(19)) << to_write << AttrOff(COLOR_PAIR(19));
		} else if (chunk.all_squares([](const auto& square) {
				return square.state == SquareState::CLOSED;
			}))
		{
			main << "   ";
		} else
		{
			main << AttrOn(COLOR_PAIR(13)) << to_write << AttrOff(COLOR_PAIR(13));
		}
	}

	void ChunkView::draw_empty_chunk(int x, int y)
	{
		main << Move({3 * x, y});
		main << "   ";
	}

	void ChunkView::draw_sidebar(ChunkSquareSource& squareSource, CursorMap& cursors) {

	}
}
