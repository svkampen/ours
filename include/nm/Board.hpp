#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cstdint>
#include <unordered_map>

#include "Typedefs.hpp"
#include "Square.hpp"
#include "Chunk.hpp"
#include "ChunkGenerator.hpp"
#include "SquareSource.hpp"
#include "ChunkSource.hpp"
#include <boost/optional.hpp>

namespace nm
{
	class Board : public SquareSource, public ChunkSource
	{
		private:
			static Chunk CHUNK_EMPTY;
			ChunkList chunks;
			ChunkGenerator chunkGenerator;

			bool client_mode = false;

		public:
			Board();
			Board(const ChunkGenerator& chunkGenerator);
			void add_chunk(const Coordinates& c, const Chunk& chunk);
			const ChunkList& get_chunks() const;
			boost::optional<Chunk&> get_chunk(const Coordinates& c);
			Chunk& regenerate_chunk(const Coordinates& c);
			void set_client_mode(bool);
			void clear_at(int x, int y);

			Square& get(const Coordinates& coordinates);
			Square& get(int x, int y);
	};
};

#endif // BOARD_HPP
