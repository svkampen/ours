#include <nm/NetworkGame.hpp>
#include <netmine.pb.h>

namespace nm
{
	NetworkGame::NetworkGame(Client& client) : board(ChunkGenerator(0.0, 0.0)), client(client)
	{

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
	void NetworkGame::open_square_handler(int x, int y)
	{
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.SQUARE_OPEN);

		auto squareOpen = wrapper.mutable_squareopen();
		squareOpen->set_x(x);
		squareOpen->set_y(y);

		client.send_message(wrapper);
	}
}
