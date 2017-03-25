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

		for (int y = 0; y < YViewable; y++)
		{
			for (int x = 0; x < XViewable; x++)
			{
				main << Move({3*x+1, y});
				auto opt_chunk = squareSource.get_chunk({x + chunk_cursor.offset_x, y + chunk_cursor.offset_y});
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

	HandlerResult ChunkView::handle_input(int input_character)
	{
		switch(input_character)
		{
			case KEY_LEFT:
				if (chunk_cursor.x > 0)
					chunk_cursor.x--;
				break;
			case KEY_RIGHT:
				if (chunk_cursor.x < (main.cols / 3) - 1)
					chunk_cursor.x++;
				break;
			case KEY_UP:
				if (chunk_cursor.y > 0)
					chunk_cursor.y--;
				break;
			case KEY_DOWN:
				if (chunk_cursor.y < main.lines - 1)
					chunk_cursor.y++;
				break;
			default:
				break;
		}

		moved = true;
		return HandlerResult::DRAW_ALL;
	}

	void ChunkView::switched_in_handler()
	{
		Coordinates chunk_coordinates = utils::to_chunk_coordinates(this->cursor.to_global());

		this->chunk_cursor.x = chunk_coordinates.x() + main.cols / 6;
		this->chunk_cursor.y = chunk_coordinates.y() + main.lines / 2;

		this->chunk_cursor.offset_x = -main.cols / 6;
		this->chunk_cursor.offset_y = -main.lines / 2;
	}

	void ChunkView::switched_out_handler()
	{
		if (!moved)
			return;
		Coordinates global_chunk = this->chunk_cursor.to_global();
		Coordinates global = utils::to_global_coordinates({8, 8}, global_chunk);

		this->cursor.offset_x = global.x()-main.cols / 6;
		this->cursor.offset_y = global.y()-main.lines / 2;

		this->cursor.x = main.cols / 6;
		this->cursor.y = main.lines / 2;

		moved = false;
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

		std::wstring to_write = (x == chunk_cursor.x && y == chunk_cursor.y) ? L"[ ]" : L"   ";

		if (chunk.all_squares([](const auto& square) {
				return square.state == SquareState::OPENED || square.state == SquareState::FLAGGED;
			}))
		{
			main << AttrOn(COLOR_PAIR(19)) << to_write << AttrOff(COLOR_PAIR(19));
		} else if (chunk.all_squares([](const auto& square) {
				return square.state == SquareState::CLOSED;
			}))
		{
			main << to_write;
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
