#ifndef _NM_GAME_H_
#define _NM_GAME_H_

#include "Board.hpp"
#include <nm/Typedefs.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Utils.hpp>

#include <optional>
#include <unordered_set>
#include <iostream>

struct open_results {
	const nm::Coordinates affected_chunk;
	const bool mine_opened;
};

namespace nm
{
	class Game
	{
		private:
			std::optional<open_results> open_square(int x, int y);
			void number_square(const Coordinates& c);
			void renumber_chunk(const Coordinates& c, const bool renumber_original);
			bool completely_flagged(int x, int y);
			bool save = false;
		public:
			Game();
			Game(Board&& board);

			Game(const Game& other);
			Game(Game&& other);

			Game& operator=(const Game& other);
			Game& operator=(Game&& other);

			~Game();
			Board board;
			void flag_square_handler(int x, int y);
			void open_square_handler(const int x, const int y, const bool check_flags = true);

			void save_on_destruct() { save = true; };

			std::unordered_set<Coordinates, int_pair_hash<Coordinates>> updated_chunks;
	};
}

#endif //_NM_GAME_H_
