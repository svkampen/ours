#include <nm/NetworkGame.hpp>
#include <netmine.pb.h>
#include <algorithm>

#include <boost/log/trivial.hpp>

namespace nm
{
	NetworkGame::NetworkGame(Client& client) : client(client), board(ChunkGenerator(0.0, 0.0)) 
	{
		board.set_client_mode(true);
		is_first_open.set();
	}

	void NetworkGame::chunk_update_handler(Client *client, const message::ChunkBytes& msg)
	{
		char *data = const_cast<char*>(msg.data().c_str());

		Chunk &chunk = board.get_chunk({msg.x(), msg.y()});
		chunk.deserialize(data);
	}

	void NetworkGame::flag_square_handler(int x, int y)
	{
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.SQUARE_FLAG);

		auto squareFlag = wrapper.mutable_squareflag();
		squareFlag->set_x(x);
		squareFlag->set_y(y);

		client.send_message(wrapper);
	}

	void NetworkGame::request_chunk(Coordinates c)
	{
		requested_chunks.push_back(c);

		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.CHUNK_REQUEST);
		auto chunkRequest = wrapper.mutable_chunkrequest();

		chunkRequest->set_x(c.x());
		chunkRequest->set_y(c.y());

		client.send_message(wrapper);
	}

	bool NetworkGame::chunk_requested(Coordinates c)
	{
		return std::find(requested_chunks.begin(), requested_chunks.end(), c) != requested_chunks.end();
	}

	Square& NetworkGame::get(Coordinates c)
	{
		Coordinates chunk_coords = to_chunk_coordinates(c);
		if (!chunk_requested(chunk_coords))
		{
			BOOST_LOG_TRIVIAL(info) << "Requesting chunk at (" << chunk_coords.x() << ", " << chunk_coords.y() << ")";
			request_chunk(chunk_coords);
		}

		return board.get(c);
	}

	Square& NetworkGame::get(int x, int y)
	{
		return this->get({x, y});
	}

	void NetworkGame::send_player_join(int x, int y)
	{
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.PLAYER_JOIN);
		auto playerJoin = wrapper.mutable_playerjoin();
		playerJoin->set_x(x);
		playerJoin->set_y(y);

		client.send_message(wrapper);
	}

	void NetworkGame::open_square_handler(int x, int y)
	{
		if (this->is_first_open())
		{
			send_player_join(x, y);
		}
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.SQUARE_OPEN);

		auto squareOpen = wrapper.mutable_squareopen();
		squareOpen->set_x(x);
		squareOpen->set_y(y);

		client.send_message(wrapper);
	}
}
