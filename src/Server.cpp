#include <nm/Server.hpp>
#include <nm/Hash.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

using boost::asio::ip::tcp;

namespace nm
{
	template<typename Event, typename Functor, typename Instance>
	void message_event_connect(Event& event, Functor functor, Instance instance) {
		event.connect(boost::bind(functor, instance, _1, _2));
	}

	namespace server
	{
		Server::Server(Game& game, int port) : connectionManager(tcp::endpoint(tcp::v4(), port)), game(game) {
			message_event_connect(connectionManager.ev_player_join, &Server::player_join_handler, this);
			message_event_connect(connectionManager.ev_player_quit, &Server::player_quit_handler, this);
			message_event_connect(connectionManager.ev_cursor_move, &Server::cursor_move_handler, this);
			message_event_connect(connectionManager.ev_square_open, &Server::square_open_handler, this);
			message_event_connect(connectionManager.ev_square_flag, &Server::square_flag_handler, this);
			message_event_connect(connectionManager.ev_chunk_request, &Server::chunk_request_handler, this);
			message_event_connect(connectionManager.ev_clear_at, &Server::clear_at_handler, this);
		}

		Server::Server(Game& game) : Server(game, 4096)
		{
		};

		void Server::player_quit_handler(Connection::ptr connection, const message::Player& _)
		{
			auto&& player = this->clients[connection];

			message::MessageWrapper wrapper;
			wrapper.set_type(wrapper.PLAYER_QUIT);
			auto playerQuit = wrapper.mutable_playerquit();
			playerQuit->CopyFrom(player);

			connectionManager.send_all_other(connection, wrapper);

			this->clients.erase(connection);
		}

		void Server::clear_at_handler(Connection::ptr connection, const message::ClearAt& clearAt)
		{
			// game.board.clear_at(clearAt.x(), clearAt.y());
		}

		void Server::send_chunk_update(int x, int y)
		{
			boost::optional<Chunk&> maybeChunk = game.board.get_chunk({x, y});
			if (!maybeChunk)
				return;

			send_chunk_update(x, y, maybeChunk.get());
		}

		void Server::send_chunk_update(int x, int y, const Chunk& chunk)
		{
			Chunk transformed_chunk = chunk.transform_copy([](Square& square){
				if (square.state == SquareState::CLOSED)
					square.is_mine = false;
			});

			char *data = transformed_chunk.serialize();

			message::MessageWrapper chunkWrapper;
			chunkWrapper.set_type(chunkWrapper.CHUNK_BYTES);
			auto cBytes = chunkWrapper.mutable_chunkbytes();
			cBytes->set_data(std::string(data, NM_CHUNK_SIZE * NM_CHUNK_SIZE));

			delete[] data;

			cBytes->set_x(x);
			cBytes->set_y(y);

			BOOST_LOG_TRIVIAL(info) << "[Server] Sending chunk update: (" << x << ", " << y << ")";

			this->connectionManager.send_all(chunkWrapper);
		}

		void Server::chunk_request_handler(Connection::ptr connection, const message::ChunkRequest& msg)
		{
			send_chunk_update(msg.x(), msg.y());
		}

		void Server::cursor_move_handler(Connection::ptr connection, const message::CursorMove& msg)
		{
			auto&& player = this->clients[connection];

			message::MessageWrapper wrapper;
			wrapper.set_type(wrapper.CURSOR_MOVE);

			auto cMove = wrapper.mutable_cursormove();
			cMove->CopyFrom(msg);
			cMove->set_id(player.id());

			/* update local (x, y) stats */
			player.set_x(msg.x());
			player.set_y(msg.y());

			this->connectionManager.send_all_other(connection, wrapper);
		}

		void Server::square_open_handler(Connection::ptr connection, const message::SquareOpen& msg)
		{
			BOOST_LOG_TRIVIAL(info) << "[Server] Received SQUARE_OPEN message: " << msg.ShortDebugString();
			game.open_square_handler(msg.x(), msg.y());

			for (const auto& coordinates : game.updated_chunks)
			{
				boost::optional<Chunk&> maybeChunk = game.board.get_chunk(coordinates);

				if (!maybeChunk)
					continue;

				this->send_chunk_update(coordinates.x(), coordinates.y(), maybeChunk.get());
			}

			game.updated_chunks.clear();
		}

		void Server::square_flag_handler(Connection::ptr connection, const message::SquareFlag& msg)
		{
			BOOST_LOG_TRIVIAL(info) << "[Server] Received SQUARE_FLAG message: " << msg.ShortDebugString();
			game.flag_square_handler(msg.x(), msg.y());

			for (const auto& coordinates : game.updated_chunks)
			{
				boost::optional<Chunk&> maybeChunk = game.board.get_chunk(coordinates);

				if (!maybeChunk)
					continue;

				this->send_chunk_update(coordinates.x(), coordinates.y(), maybeChunk.get());
			}

			game.updated_chunks.clear();

		}

		void Server::player_join_handler(Connection::ptr connection, const message::Player& msg)
		{
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

			message::MessageWrapper wrapper;
			wrapper.set_type(wrapper.PLAYER_JOIN);

			auto pJoin = wrapper.mutable_playerjoin();
			pJoin->CopyFrom(msg);
			pJoin->set_id(hash);

			this->connectionManager.send_all_other(connection, wrapper);

			wrapper.Clear();

			/* And send a welcome message to the newly joined client */

			auto welcome = wrapper.mutable_welcome();

			welcome->set_version(1);
			welcome->set_nplayers(this->clients.size() - 1);
			wrapper.set_type(wrapper.WELCOME);

			int i = 0;
			for (auto iterator = this->clients.begin(); iterator != this->clients.end(); iterator++, i++)
			{
				if (iterator->first != connection)
				{
					auto playerdata = welcome->add_players();
					playerdata->CopyFrom(iterator->second);
				}
			}

			connection->sendMessage(wrapper);
		}

		void Server::start()
		{
			connectionManager.start();
		}
	}
}
