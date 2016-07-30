#include <nm/Chunk.hpp>
#include <algorithm>

namespace nm
{
	Chunk Chunk::CHUNK_EMPTY = {};

	Square& Chunk::get(const Coordinates& coordinates)
	{
		return chunk[coordinates.y()][coordinates.x()];
	}

	Square& Chunk::get(int x, int y)
	{
		return chunk[y][x];
	}

	const Square& Chunk::get(const Coordinates& coordinates) const
	{
		return chunk[coordinates.y()][coordinates.x()];
	}

	const Square& Chunk::get(const int x, const int y) const
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


	void Chunk::transform(const Chunk::SquareFn& functor)
	{
		std::for_each(chunk.begin(), chunk.end(), [&functor](std::array<Square, NM_CHUNK_SIZE>& row)
		{
			std::for_each(row.begin(), row.end(), functor);
		});
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
