#ifndef NM_NETWORKGAME_HPP
#define NM_NETWORKGAME_HPP

#include <nm/Client.hpp>
#include <nm/Board.hpp>
#include <nm/ChunkGenerator.hpp>
#include <nm/Flag.hpp>
#include <nm/ChunkSquareSource.hpp>
#include <netmine.pb.h>

#include <vector>

#include <boost/signals2.hpp>
#include <optional>

namespace nm
{
	class NetworkGame : public ChunkSquareSource
	{
		private:
			Client& client;
			std::vector<Coordinates> requested_chunks;
			Flag is_first_open;
			void send_player_join(int x, int y);
			void send_clear_at(int x, int y);
			void request_chunk(const Coordinates& c);
			bool chunk_requested(const Coordinates& c);

			bool connected = false;
		public:
			NetworkGame(Client& client);
			Board board;

			boost::signals2::signal<void ()> ev_board_update;
			boost::signals2::signal<void (const message::Player&)> ev_new_player;

			Square& get(const Coordinates& c);
			Square& get(int x, int y);
            const Square& get(const Coordinates& c) const;
            const Square& get(int x, int y) const;

			const ChunkList& get_chunks() const;
			std::optional<Chunk* const> get_chunk(const Coordinates& coordinates);

			void connected_handler();
			void save_image_handler(std::string args);
			void welcome_handler(const message::MessageWrapper& mwpr);
			void chunk_update_handler(const message::MessageWrapper& mwpr);
			void cursor_move_handler(int x, int y);
			void flag_square_handler(int x, int y);
			void open_square_handler(int x, int y);
	};
}

#endif
