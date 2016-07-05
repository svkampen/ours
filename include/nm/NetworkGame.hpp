#ifndef NM_NETWORKGAME_HPP
#define NM_NETWORKGAME_HPP

#include <nm/Client.hpp>
#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>
#include <netmine.pb.h>

namespace nm
{
	class NetworkGame
	{
		private:
			Client& client;
		public:
			NetworkGame(Client& client);
			Board board;
			void chunk_update_handler(Client *client, const message::ChunkBytes& msg);
			void flag_square_handler(int x, int y);
			void open_square_handler(int x, int y);
	};
}

#endif
