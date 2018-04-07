#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>

#include <iostream>
#include <cmath>

#include <boost/log/trivial.hpp>
#include <nm/Utils.hpp>
#include <cassert>

namespace nm
{
	constexpr int32_t modulo(const int32_t n, const int32_t mod)
	{
		// Since C++'s modulo behavior with negative numbers is dependent on the
		// implementation, this function makes sure modulo behavior is consistent
		// across implementations.
		int32_t temp = n % mod;
		if (temp < 0)
			return mod + temp;
		return temp;
	}

	Board::Board(const ChunkGenerator& chunkGenerator) : chunkGenerator(chunkGenerator)
	{
	};

	Board::Board() : Board(ChunkGenerator(0.15, 0.03))
	{
        add_chunk({0, 0}, chunkGenerator.generate());
	};

	void Board::add_chunk(const Coordinates& c, const Chunk& chunk)
	{
		chunks[c] = chunk;
	}

	std::optional<Chunk* const> Board::get_chunk(const Coordinates& c)
	{
		auto iter = chunks.find(c);
		if (iter == chunks.end())
			return std::optional<Chunk* const>();
		else
			return std::optional<Chunk* const>(&iter->second);
	}

	std::optional<Chunk* const> Board::get_chunk(int x, int y)
	{
		return get_chunk({x, y});
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
		Coordinates local_coords = {modulo(x, NM_CHUNK_SIZE), modulo(y, NM_CHUNK_SIZE)};

		Coordinates chunk_coordinates = nm::utils::to_chunk_coordinates({x, y});

		BOOST_LOG_TRIVIAL(info) << "Clearing around (X: " << local_coords.x() << " Y: " << local_coords.y() << "), "
			<< "Chunk " << chunk_coordinates.x() << ", " << chunk_coordinates.y() << ".";

		std::optional<Chunk* const> maybeChunk = this->get_chunk(chunk_coordinates);
		Chunk* const chunk = maybeChunk ? *maybeChunk : &regenerate_chunk(chunk_coordinates);

		static int around_offsets[3] = {-1, 0, 1};

		for (auto&& xoff : around_offsets)
		{
			for (auto&& yoff : around_offsets)
			{
				Square &around = chunk->get(local_coords.x() + xoff, local_coords.y() + yoff);
				around.is_mine = false;
			}
		}
	}

    /* Doing this properly is probably possible. */
    const Square& Board::get(const Coordinates& coordinates) const
    {
        if (client_mode)
            return const_cast<Board*>(this)->get(coordinates);
        else
            throw std::bad_cast();
    }

    const Square& Board::get(int x, int y) const
    {
        return get({x, y});
    }

	Square& Board::get(const Coordinates& coordinates)
	{
		Coordinates local_coords = {modulo(coordinates.x(), NM_CHUNK_SIZE), modulo(coordinates.y(), NM_CHUNK_SIZE)};

		Coordinates chunk_coordinates = {static_cast<int>(std::floor(coordinates.x() / (double)NM_CHUNK_SIZE)),
										 static_cast<int>(std::floor(coordinates.y() / (double)NM_CHUNK_SIZE))};

		auto chunk_iterator = chunks.find(chunk_coordinates);
		if (chunk_iterator == chunks.end())
		{
			if (client_mode)
				return Chunk::CHUNK_EMPTY.get(local_coords);
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
