#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>

#include <iostream>
#include <cmath>

#include <boost/log/trivial.hpp>
#include <nm/Utils.hpp>
#include <cassert>

namespace nm
{
	const int32_t modulo(const int32_t n, const int32_t mod)
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

	Chunk Board::CHUNK_EMPTY = {};

	Board::Board(const ChunkGenerator& chunkGenerator) : chunkGenerator(chunkGenerator)
	{
	};

	Board::Board() : Board(ChunkGenerator(0.20, 0.03))
	{
	};

	void Board::add_chunk(const Coordinates& c, const Chunk& chunk)
	{
		chunks[c] = chunk;
	}

	boost::optional<Chunk&> Board::get_chunk(const Coordinates& c)
	{
		auto iter = chunks.find(c);
		if (iter == chunks.end())
			return boost::optional<Chunk&>();
		else
			return boost::optional<Chunk&>(iter->second);
	}

	const ChunkList& Board::get_chunks() const
	{
		return this->chunks;
	}

	void Board::set_client_mode(bool b)
	{
		client_mode = b;
	}

	Chunk& Board::regenerate_chunk(const Coordinates& c)
	{
		return chunks[c] = chunkGenerator.generate();
	}

	void Board::clear_at(int x, int y)
	{
		simmo::vector<int32_t, 2> local_coords = {modulo(x, NM_CHUNK_SIZE),
												  modulo(y, NM_CHUNK_SIZE)};

		Coordinates chunk_coordinates = nm::utils::to_chunk_coordinates({x, y});

		BOOST_LOG_TRIVIAL(info) << "Clearing around (X: " << local_coords.x() << " Y: " << local_coords.y() << "), "
			<< "Chunk " << chunk_coordinates.x() << ", " << chunk_coordinates.y() << ".";

		boost::optional<Chunk&> maybeChunk = this->get_chunk(chunk_coordinates);
		Chunk& chunk = maybeChunk.is_initialized() ? maybeChunk.get() : regenerate_chunk(chunk_coordinates);

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

	Square& Board::get(const Coordinates& coordinates)
	{
		simmo::vector<int32_t, 2> local_coords = {modulo(coordinates.x(), NM_CHUNK_SIZE),
												  modulo(coordinates.y(), NM_CHUNK_SIZE)};

		Coordinates chunk_coordinates = {static_cast<int>(std::floor(coordinates.x() / (double)NM_CHUNK_SIZE)),
										 static_cast<int>(std::floor(coordinates.y() / (double)NM_CHUNK_SIZE))};

		auto chunk_iterator = chunks.find(chunk_coordinates);
		if (chunk_iterator == chunks.end())
		{
			if (client_mode)
				return CHUNK_EMPTY.get(local_coords);
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
