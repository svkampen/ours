#ifndef NM_NETWORKGAME_HPP
#define NM_NETWORKGAME_HPP

#include <nm/Client.hpp>
#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>
#include <nm/Flag.hpp>
#include <netmine.pb.h>

#include <vector>

#include <boost/signals2.hpp>

namespace nm
{
	class NetworkGame : public SquareSource
	{
		private:
			Client& client;
			std::vector<Coordinates> requested_chunks;
			Flag is_first_open;
			void send_player_join(int x, int y);
			void send_clear_at(int x, int y);
			void request_chunk(const Coordinates& c);
			bool chunk_requested(const Coordinates& c);
		public:
			NetworkGame(Client& client);
			Board board;

			boost::signals2::signal<void ()> ev_board_update;
			boost::signals2::signal<void (message::Player&)> ev_new_player;

			virtual Square& get(const Coordinates& c);
			virtual Square& get(int x, int y);
			void connected_handler();
			void welcome_handler(const message::Welcome& welcome);
			void chunk_update_handler(const message::ChunkBytes& msg);
			void cursor_move_handler(int x, int y);
			void flag_square_handler(int x, int y);
			void open_square_handler(int x, int y);
	};
}

#endif
