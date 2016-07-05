#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>

#include <iostream>
#include <cmath>

#include <boost/log/trivial.hpp>

namespace nm
{
	int32_t modulo(int32_t n, int32_t mod)
	{
		// Since C++'s modulo behavior with negative numbers is dependent on the
		// implementation, this function makes sure modulo behavior is consistent
		// across implementations.
		int32_t temp = n % mod;
		if (temp < 0)
			return mod + temp;
		else
			return temp;
	}

	Board::Board(ChunkGenerator chunkGenerator) : chunkGenerator(chunkGenerator)
	{
	};

	Board::Board() : Board(ChunkGenerator(0.15, 0.03))
	{
	};

	Chunk& Board::get_chunk(Coordinates c)
	{
		return chunks[c];
	}

	void Board::clear_at(int x, int y)
	{
		simmo::vector<int32_t, 2> local_coords = {modulo(x, NM_CHUNK_SIZE),
												  modulo(y, NM_CHUNK_SIZE)};

		Coordinates chunk_coordinates = {static_cast<int>(std::floor(x / static_cast<double>(NM_CHUNK_SIZE))),
										 static_cast<int>(std::floor(y / static_cast<double>(NM_CHUNK_SIZE)))};

		Chunk& chunk = chunks[chunk_coordinates];
		static int around_offsets[3] = {-1, 0, 1};

		for (auto&& xoff : around_offsets)
		{
			for (auto&& yoff : around_offsets)
			{
				Square &around = chunk.get(local_coords.x() + xoff, local_coords.y() + yoff);
				around.is_mine = false;
			}
		}
	}

	Square& Board::get(Coordinates coordinates)
	{
		simmo::vector<int32_t, 2> local_coords = {modulo(coordinates.x(), NM_CHUNK_SIZE),
												  modulo(coordinates.y(), NM_CHUNK_SIZE)};

		Coordinates chunk_coordinates = {static_cast<int>(std::floor(coordinates.x() / (double)NM_CHUNK_SIZE)),
										 static_cast<int>(std::floor(coordinates.y() / (double)NM_CHUNK_SIZE))};

		auto chunk_iterator = chunks.find(chunk_coordinates);
		if (chunk_iterator == chunks.end())
		{
			// The chunk does not exist, so we generate it.
			BOOST_LOG_TRIVIAL(info) << "[board] generating chunk at (" << chunk_coordinates.x()
					  << ", " << chunk_coordinates.y() << ")";
			chunks[chunk_coordinates] = chunkGenerator.generate();
		}

		return chunks[chunk_coordinates].get(local_coords);
	}

	Square& Board::get(int x, int y)
	{
		return this->get({x, y});
	}
}
