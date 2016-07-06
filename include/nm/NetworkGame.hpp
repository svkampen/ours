#ifndef NM_NETWORKGAME_HPP
#define NM_NETWORKGAME_HPP

#include <nm/Client.hpp>
#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>
#include <nm/Flag.hpp>
#include <netmine.pb.h>

#include <vector>

namespace nm
{
	class NetworkGame : public SquareSource
	{
		private:
			Client& client;
			std::vector<Coordinates> requested_chunks;
			Flag is_first_open;
			void send_player_join(int x, int y);
			void request_chunk(Coordinates c);
			bool chunk_requested(Coordinates c);
		public:
			NetworkGame(Client& client);
			Board board;
			virtual Square& get(Coordinates c);
			virtual Square& get(int x, int y);
			void chunk_update_handler(Client *client, const message::ChunkBytes& msg);
			void flag_square_handler(int x, int y);
			void open_square_handler(int x, int y);
	};
}

#endif
