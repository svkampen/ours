#ifndef _NM_GAME_H_
#define _NM_GAME_H_

#include "Board.hpp"
#include <nm/Typedefs.hpp>
#include <nm/SquareSource.hpp>

#include <unordered_set>

struct open_results {
	const nm::Coordinates affected_chunk;
	const bool mine_opened;
};

namespace nm
{
	class Game
	{
		private:
			struct open_results open_square(int x, int y);
			void number_square(const Coordinates& c);
			void renumber_chunk(const Coordinates& c, const bool renumber_original);
			bool completely_flagged(int x, int y);
		public:
			Game();
			Game(Board board);
			Board board;
			void flag_square_handler(int x, int y);
			void open_square_handler(const int x, const int y, const bool check_flags = true);

			std::unordered_set<Coordinates> updated_chunks;
	};
}

#endif //_NM_GAME_H_
