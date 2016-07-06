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

namespace nm
{
	class Board : public SquareSource
	{
		private:
			static Chunk CHUNK_EMPTY;
			std::unordered_map<Coordinates, Chunk> chunks;
			ChunkGenerator chunkGenerator;

			bool client_mode = false;

		public:
			Board();
			Board(ChunkGenerator chunkGenerator);
			void add_chunk(Coordinates c, Chunk chunk);
			Chunk& get_chunk(Coordinates c);
			void set_client_mode(bool);
			void clear_at(int x, int y);
			Square& get(Coordinates coordinates);
			Square& get(int x, int y);
	};
};

#endif // BOARD_HPP
