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
		Server::Server() : connectionManager(tcp::endpoint(tcp::v4(), 4096))
		{
			message_event_connect(connectionManager.ev_player_join, &Server::player_join_handler, this);
			message_event_connect(connectionManager.ev_cursor_move, &Server::cursor_move_handler, this);
			message_event_connect(connectionManager.ev_square_open, &Server::square_open_handler, this);
			message_event_connect(connectionManager.ev_square_flag, &Server::square_flag_handler, this);
			message_event_connect(connectionManager.ev_chunk_request, &Server::chunk_request_handler, this);
		}

		void Server::send_chunk_update(int x, int y)
		{
			boost::optional<Chunk&> maybeChunk = game.board.get_chunk({x, y});
			if (!maybeChunk)
				return;

			send_chunk_update(x, y, maybeChunk.get());
		}

		void Server::send_chunk_update(int x, int y, Chunk &chunk)
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
			auto id_iter = this->clients.left.find(connection);

			message::MessageWrapper wrapper;
			wrapper.set_type(wrapper.CURSOR_MOVE);

			auto cMove = wrapper.mutable_cursormove();
			cMove->CopyFrom(msg);
			cMove->set_id(id_iter->second);

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
			BOOST_LOG_TRIVIAL(info) << "[Server] Received SQUARE_FLAG message: " << msg.DebugString();
			game.flag_square_handler(msg.x(), msg.y());

			int x = ((int)std::floor(msg.x() / (double)NM_CHUNK_SIZE));
			int y = ((int)std::floor(msg.y() / (double)NM_CHUNK_SIZE));

			this->send_chunk_update(x, y);
		}

		void Server::player_join_handler(Connection::ptr connection, const message::Player& msg)
		{
			auto endpoint = connection->socket().remote_endpoint();
			std::string endpoint_and_port = endpoint.address().to_string() + std::to_string(endpoint.port());

			int32_t hash = hash::Hash(endpoint_and_port.c_str());

			this->clients.insert(client_pair(connection, hash));
			BOOST_LOG_TRIVIAL(info) << "[Server] received PLAYER_JOIN message (X: " << msg.x()
				<< " Y: " << msg.y() << " ID: " << hash << ")";

			game.board.clear_at(msg.x(), msg.y());

			message::MessageWrapper wrapper;
			wrapper.set_type(wrapper.PLAYER_JOIN);

			auto pJoin = wrapper.mutable_playerjoin();
			pJoin->CopyFrom(msg);
			pJoin->set_id(hash);

			this->connectionManager.send_all_other(connection, wrapper);

			wrapper.Clear();

			auto welcome = wrapper.mutable_welcome();

			welcome->set_version(1);
			welcome->set_nplayers(this->clients.size() - 1);
			wrapper.set_type(wrapper.WELCOME);

			connection->sendMessage(wrapper);
		}

		void Server::start()
		{
			connectionManager.start();
		}
	}
}
