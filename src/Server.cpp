#include <nm/Server.hpp>
#include <nm/Hash.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

using boost::asio::ip::tcp;

namespace nm
{
	namespace server
	{
		Server::Server(Game& game, int port) : connectionManager(tcp::endpoint(tcp::v4(), port)), game(game) {
			connectionManager.event_map.connect(MessageType(PLAYER_JOIN),
				boost::bind(&Server::player_join_handler, this, _1, _2));

			connectionManager.event_map.connect(MessageType(PLAYER_QUIT),
				boost::bind(&Server::player_quit_handler, this, _1, _2));

			connectionManager.event_map.connect(MessageType(CURSOR_MOVE),
				boost::bind(&Server::cursor_move_handler, this, _1, _2));

			connectionManager.event_map.connect(MessageType(SQUARE_OPEN),
				boost::bind(&Server::square_open_handler, this, _1, _2));

			connectionManager.event_map.connect(MessageType(SQUARE_FLAG),
				boost::bind(&Server::square_flag_handler, this, _1, _2));

			connectionManager.event_map.connect(MessageType(CHUNK_REQUEST),
				boost::bind(&Server::chunk_request_handler, this, _1, _2));

			connectionManager.event_map.connect(MessageType(CLEAR_AT),
				boost::bind(&Server::clear_at_handler, this, _1, _2));
		}

		Server::Server(Game& game) : Server(game, 4096)
		{
		};

		void Server::player_quit_handler(Connection::ptr connection, const message::MessageWrapper& wrapper)
		{
			connectionManager.send_all_other(connection, wrapper);

			this->clients.erase(connection);
		}

		void Server::clear_at_handler(Connection::ptr connection, const message::MessageWrapper& clearAt)
		{
			// game.board.clear_at(clearAt.x(), clearAt.y());
		}

		void Server::send_chunk_update(int x, int y)
		{
			std::optional<Chunk* const> maybeChunk = game.board.get_chunk({x, y});
			if (!maybeChunk)
				return;

			send_chunk_update(x, y, **maybeChunk);
		}

		void Server::send_chunk_update(int x, int y, const Chunk& chunk)
		{
			Chunk transformed_chunk = chunk.transform_copy([](Square& square){
				if (square.state == SquareState::CLOSED)
					square.is_mine = false;
			});

			auto data = transformed_chunk.serialize();

			message::MessageWrapper chunkWrapper;
			chunkWrapper.set_type(chunkWrapper.CHUNK_BYTES);
			auto cBytes = chunkWrapper.mutable_chunkbytes();
			cBytes->set_data(std::string(data.get(), NM_CHUNK_SIZE * NM_CHUNK_SIZE));

			cBytes->set_x(x);
			cBytes->set_y(y);

			BOOST_LOG_TRIVIAL(info) << "[Server] Sending chunk update: (" << x << ", " << y << ")";

			this->connectionManager.send_all(chunkWrapper);
		}

		void Server::chunk_request_handler(Connection::ptr connection, const message::MessageWrapper& wrapper)
		{
			auto& msg = wrapper.chunkrequest();
			send_chunk_update(msg.x(), msg.y());
		}

		void Server::cursor_move_handler(Connection::ptr connection, const message::MessageWrapper& wrapper)
		{
			auto&& player = this->clients[connection];

			message::MessageWrapper downstream = wrapper;
			auto cMove = downstream.mutable_cursormove();
			cMove->set_id(player.id());

			/* update local (x, y) stats */
			player.set_x(cMove->x());
			player.set_y(cMove->y());

			this->connectionManager.send_all_other(connection, downstream);
		}

		void Server::square_open_handler(Connection::ptr connection, const message::MessageWrapper& wrapper)
		{
			auto& msg = wrapper.squareopen();
			BOOST_LOG_TRIVIAL(info) << "[Server] Received SQUARE_OPEN message: " << msg.ShortDebugString();
			game.open_square_handler(msg.x(), msg.y());

			for (const auto& coordinates : game.updated_chunks)
			{
				std::optional<Chunk* const> maybeChunk = game.board.get_chunk(coordinates);

				if (!maybeChunk)
					continue;

				this->send_chunk_update(coordinates.x(), coordinates.y(), **maybeChunk);
			}

			game.updated_chunks.clear();
		}

		void Server::square_flag_handler(Connection::ptr connection, const message::MessageWrapper& wrapper)
		{
			auto& msg = wrapper.squareflag();
			BOOST_LOG_TRIVIAL(info) << "[Server] Received SQUARE_FLAG message: " << msg.ShortDebugString();
			game.flag_square_handler(msg.x(), msg.y());

			for (const auto& coordinates : game.updated_chunks)
			{
				std::optional<Chunk* const> maybeChunk = game.board.get_chunk(coordinates);

				if (!maybeChunk)
					continue;

				this->send_chunk_update(coordinates.x(), coordinates.y(), **maybeChunk);
			}

			game.updated_chunks.clear();

		}

		void Server::player_join_handler(Connection::ptr connection, const message::MessageWrapper& wrapper)
		{
			auto& msg = wrapper.player();
			/* Generate an ID and add an entry in the client dict. */
			auto endpoint = connection->socket().remote_endpoint();
			std::string endpoint_and_port = endpoint.address().to_string() + std::to_string(endpoint.port());

			int32_t hash = hash::Hash(endpoint_and_port.c_str());

			message::Player player;
			player.set_x(msg.x());
			player.set_y(msg.y());
			player.set_id(hash);

			this->clients[connection] = player;
			BOOST_LOG_TRIVIAL(info) << "[Server] received PLAYER_JOIN message (X: " << msg.x()
				<< " Y: " << msg.y() << " ID: " << player.id() << ")";

			/* Then, send the playerjoin (including ID) to every other client */

			message::MessageWrapper downstream;
			downstream.set_type(downstream.PLAYER_JOIN);

			auto pJoin = downstream.mutable_player();
			pJoin->CopyFrom(msg);
			pJoin->set_id(hash);

			this->connectionManager.send_all_other(connection, downstream);

			downstream.Clear();

			/* And send a welcome message to the newly joined client */

			auto welcome = downstream.mutable_welcome();

			welcome->set_version(1);
			welcome->set_nplayers(this->clients.size() - 1);
			downstream.set_type(downstream.WELCOME);

			int i = 0;
			for (auto iterator = this->clients.begin(); iterator != this->clients.end(); iterator++, i++)
			{
				if (iterator->first != connection)
				{
					auto playerdata = welcome->add_players();
					playerdata->CopyFrom(iterator->second);
				}
			}

			connection->sendMessage(downstream);

			/* Also send all known current chunks. */
			const auto& chunkList = this->game.board.get_chunks();

			for (const auto& chunkPair : chunkList)
			{
				this->send_chunk_update(chunkPair.first.x(), chunkPair.first.y(), chunkPair.second);
			}
		}

		void Server::start()
		{
			connectionManager.start();
		}
	}
}
