#ifndef _NM_GAME_H_
#define _NM_GAME_H_

#include "Board.hpp"
#include <nm/Typedefs.hpp>

#include <unordered_set>

namespace nm
{
	class Game
	{
		private:
			Coordinates open_square(int x, int y);
		public:
			Board board;
			void flag_square_handler(int x, int y);
			std::unordered_set<Coordinates> open_square_recursive(int x, int y);
			void open_square_handler(int x, int y, std::function<void(int, int, Chunk&)> functor);
	};
}

#endif //_NM_GAME_H_
