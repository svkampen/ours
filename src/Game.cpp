#include <nm/Game.hpp>
#include <vector>

namespace nm
{
	Coordinates Game::open_square(int x, int y)
	{
		Square &square = board.get({x, y});
		Coordinates chunk_coordinates = {static_cast<int>(std::floor(x / static_cast<double>(NM_CHUNK_SIZE))),
										 static_cast<int>(std::floor(y / static_cast<double>(NM_CHUNK_SIZE)))};

		square.number = 0;

		static int around_offsets[3] = {-1, 0, 1};
		for (auto&& xoff : around_offsets)
		{
			for (auto&& yoff : around_offsets)
			{
				if (board.get({x+xoff, y+yoff}).is_mine)
					square.number++;
			}
		}

		square.state = SquareState::OPENED;
		return chunk_coordinates;
	}

	void Game::flag_square_handler(int x, int y)
	{
		Square &square = board.get(x, y);

		if (square.state == SquareState::OPENED)
		{
			// Can't flag an opened square
			return;
		}

		if (square.state == SquareState::FLAGGED)
		{
			square.state =  SquareState::CLOSED;
		} else
		{
			square.state = SquareState::FLAGGED;
		}
	}

	void Game::open_square_handler(int x, int y, std::function<void (int, int, Chunk&)> functor)
	{
		if (this->initialized == false)
		{
			this->initialized = true;
			board.clear_at(x, y);
		}

		std::vector<Coordinates> affected_chunks;

		Square &square = board.get(x, y);
		static int around_offsets[3] = {-1, 0, 1};

		if (square.state == SquareState::FLAGGED || square.state == SquareState::OPENED)
		{
			// Do nothing, as flagged squares cannot be opened
			// and opened squares are already opened
			return;
		}

		if (affected_chunks.push_back(open_square(x, y)), square.number == 0)
		{
			for (auto&& xoff : around_offsets)
			{
				for (auto&& yoff : around_offsets)
				{
					open_square_handler(x+xoff, y+yoff, functor);
				}
			}
		}

		for (auto coordinates : affected_chunks)
		{
			Chunk &chunk = board.get_chunk(coordinates);
			functor(coordinates.x(), coordinates.y(), chunk);
		}
	}
}
