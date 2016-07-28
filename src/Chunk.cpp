#include <nm/Chunk.hpp>
#include <algorithm>

namespace nm
{
	Square& Chunk::get(const Coordinates& coordinates)
	Coordinates to_chunk_coordinates(Coordinates c)
	{
		return chunk[coordinates.y()][coordinates.x()];
	}
		int x = ((int)std::floor(c.x() / (double)NM_CHUNK_SIZE));
		int y = ((int)std::floor(c.y() / (double)NM_CHUNK_SIZE));

		return {x, y};
	}

	{
		return chunk[coordinates.y()][coordinates.x()];
	}

	Square& Chunk::get(int x, int y)
	{
		return chunk[y][x];
	}

	char *Chunk::serialize() const
	{
		char *data = new char[NM_CHUNK_SIZE * NM_CHUNK_SIZE];

		for (int x = 0; x < NM_CHUNK_SIZE; x++)
		{
			for (int y = 0; y < NM_CHUNK_SIZE; y++)
			{
				const Square &square = chunk[x][y];
				char sq_data = *reinterpret_cast<const char*>(&square);
				data[x * NM_CHUNK_SIZE + y] = sq_data;
			}
		}

		return data;
	}

	void Chunk::deserialize(const char *data) {
		for (int x = 0; x < NM_CHUNK_SIZE; x++)
		{
			for (int y = 0; y < NM_CHUNK_SIZE; y++)
			{
				Square &square = chunk[x][y];
				char *sq_data = reinterpret_cast<char*>(&square);
				*sq_data = data[x * NM_CHUNK_SIZE + y];
			}
		}
	}

	Chunk Chunk::transform_copy(const Chunk::SquareFn& functor) const
	{
		Chunk temp = *this;
		std::for_each(temp.chunk.begin(), temp.chunk.end(), [&functor](std::array<Square, NM_CHUNK_SIZE>& row)
		{
			std::for_each(row.begin(), row.end(), functor);
		});
		return temp;
	}
}
