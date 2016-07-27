#ifndef NM_SERVER_HPP
#define NM_SERVER_HPP

#include "ConnectionManager.hpp"
#include <boost/bimap.hpp>
#include <nm/Game.hpp>
#include <nm/ClientData.hpp>

namespace std
{
	template<>
	struct hash<nm::message::Player>
	{
		typedef nm::message::Player argument_type;
		std::size_t operator()(argument_type const& c) const
		{
			std::size_t const h1(std::hash<int32_t>()(c.x()));
			std::size_t const h2(std::hash<int32_t>()(c.y()));
			std::size_t const h3(std::hash<int32_t>()(c.id()));

			return (h1 ^ (h2 + h3));
		}
	};
}

namespace nm
{
namespace server
{
	class Server
	{
		public:
			Server(Game &game);
			Server(Game &game, int port);
			void start();

			/* event handlers */
			void chunk_request_handler(Connection::ptr connection, const message::ChunkRequest& msg);
			void player_join_handler(Connection::ptr connection, const message::Player& msg);
			void cursor_move_handler(Connection::ptr connection, const message::CursorMove& msg);
			void square_open_handler(Connection::ptr connection, const message::SquareOpen& msg);
			void square_flag_handler(Connection::ptr connection, const message::SquareFlag& msg);
			void clear_at_handler(Connection::ptr connection, const message::ClearAt& clearAt);
			void player_quit_handler(Connection::ptr connection, const message::Player& player);

		private:
			ConnectionManager connectionManager;
			std::unordered_map<Connection::ptr, message::Player> clients{};
			Game &game;

			void send_chunk_update(int x, int y, const Chunk& chunk);
			void send_chunk_update(int x, int y);
	};
}
}

#endif
