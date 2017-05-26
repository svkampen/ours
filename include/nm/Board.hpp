#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cstdint>
#include <unordered_map>
#include <optional>

#include "Typedefs.hpp"
#include <iostream>
#include "Square.hpp"
#include "Chunk.hpp"
#include "ChunkGenerator.hpp"
#include "ChunkSquareSource.hpp"
#include "ChunkSource.hpp"
#include "Utils.hpp"

namespace nm
{
	/**
	 * The representation of a minesweeper board.
	 * 
	 * This is simply a data structure with some functions
	 * for accessing specific coordinates in the board,
	 * and generating its own data using a specified chunk generator.
	 *
	 */
	class Board : public ChunkSquareSource 
	{
		friend class Loader;
		friend class Saver;
		friend class ImageSaver;

		private:
			ChunkList chunks;
			ChunkGenerator chunkGenerator;

			bool client_mode = false;

		public:
			/**
			 * The default constructor, uses standard chunk generator settings.
			 */
			Board();

			/**
			 * The constructor that allows different chunk generation settings.
			 */
			Board(const ChunkGenerator& chunkGenerator);

			Board(Board&& other) : chunks(std::move(other.chunks)), chunkGenerator(std::move(other.chunkGenerator))
			{
			};

			Board& operator=(Board&& other)
			{
				chunks = std::move(other.chunks);
				chunkGenerator = std::move(other.chunkGenerator);
				return *this;
			}

			Board(const Board& other) : chunks(other.chunks), chunkGenerator(other.chunkGenerator)
			{
			}

			Board& operator=(const Board& other)
			{
				chunks = other.chunks;
				chunkGenerator = other.chunkGenerator;
				return *this;
			}

			/**
			 * Add a chunk to the chunk list.
			 *
			 * @param c The coordinates of the new chunk, in chunk coordinate format.
			 * @param chunk The chunk to insert.
			 */
			void add_chunk(const Coordinates& c, const Chunk& chunk);

			/**
			 * Get the list of chunks.
			 *
			 * @return A reference to the private chunk list.
			 */
			const ChunkList& get_chunks() const;

			/**
			 * Get a specific chunk from the chunk list.
			 *
			 * @param c The coordinates of the chunk, in chunk coordinate format.
			 * @return An optional<Chunk* const>, since a chunk with the specified coordinates may not exist.
			 *  
			 */
			std::optional<Chunk* const> get_chunk(const Coordinates& c);
			std::optional<Chunk* const> get_chunk(int x, int y);

			/**
			 * Regenerate a chunk, keeping its coordinates.
			 *
			 * @param c The coordinates of the chunk, in chunk coordinate format. 
			 * @return A reference to the regenerated chunk.
			 */
			Chunk& regenerate_chunk(const Coordinates& c);

			/**
			 * Set the 'client mode' of the current board.
			 *
			 * If client mode is set, the board will not generate its own chunks when they
			 * are missing, but rather return an empty optional<Chunk&>, whereafter the
			 * chunk can be requested from the server.
			 *
			 * @param b The client mode flag.
			 */

			void set_client_mode(bool b);

			/**
			 * Clear a 3x3 square around the x, y coordinates.\ Used on first square open.
			 *
			 * @param x The x-coordinate, in global coordinate format.
			 * @param y The y-coordinate, in global coordinate format.
			 *
			 */
			void clear_at(int x, int y);

			/**
			 * Get a board square.
			 *
			 * @param coordinates The square coordinates, in global coordinate format.
			 * @return A reference to the requested square.
			 */
			Square& get(const Coordinates& coordinates);

			/**
			 * Get a board square.
			 *
			 * @todo Re-evaluate why we have the x,y functions and the Coordinates functions.
			 * @param x The x-coordinate, in global coordinate format.
			 * @param y The y-coordinate, in global coordinate format.
			 * @return A reference to the requested square.
			 */
			Square& get(int x, int y);
	};
};

#endif // BOARD_HPP
