#include <nm/NetworkGame.hpp>
#include <netmine.pb.h>
#include <algorithm>
#include <nm/ImageSaver.hpp>

#include <boost/log/trivial.hpp>
#include <nm/Utils.hpp>

namespace nm
{
	NetworkGame::NetworkGame(Client& client) : client(client), board(ChunkGenerator(0.0, 0.0))
	{
		board.set_client_mode(true);
		is_first_open.set();

		client.ev_connected.connect(boost::bind(&NetworkGame::connected_handler, this));
		client.ev_welcome.connect(boost::bind(&NetworkGame::welcome_handler, this, _1));
	}

	void NetworkGame::connected_handler()
	{
		this->send_player_join(0,0);
		this->ev_board_update();
	}

	void NetworkGame::chunk_update_handler(const message::ChunkBytes& msg)
	{
		char *data = const_cast<char*>(msg.data().c_str());

		Chunk chunk;
		chunk.deserialize(data);

		Coordinates coordinates = {msg.x(), msg.y()};

		board.add_chunk(coordinates, chunk);

		if (!chunk_requested(coordinates))
			requested_chunks.push_back(coordinates);

		this->ev_board_update();
	}

	void NetworkGame::save_image_handler()
	{
		ImageSaver saver(board);
		saver.save("board.png");
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

	void NetworkGame::welcome_handler(const message::Welcome& welcome)
	{
		for (int i = 0; i < welcome.nplayers(); i++)
		{
			auto player = welcome.players(i);
			BOOST_LOG_TRIVIAL(info) << "Adding player with ID " << player.id();
			this->ev_new_player(player);
		}
	}

	void NetworkGame::cursor_move_handler(int x, int y)
	{
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.CURSOR_MOVE);

		auto cursorMove = wrapper.mutable_cursormove();
		cursorMove->set_x(x);
		cursorMove->set_y(y);

		client.send_message(wrapper);
	}

	void NetworkGame::request_chunk(const Coordinates& c)
	{
		requested_chunks.push_back(c);

		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.CHUNK_REQUEST);
		auto chunkRequest = wrapper.mutable_chunkrequest();

		chunkRequest->set_x(c.x());
		chunkRequest->set_y(c.y());

		BOOST_LOG_TRIVIAL(info) << "Requesting chunk at (" << c.x() << ", " << c.y() << ")";
		client.send_message(wrapper);
	}

	bool NetworkGame::chunk_requested(const Coordinates& c)
	{
		return std::find(requested_chunks.begin(), requested_chunks.end(), c) != requested_chunks.end();
	}

	Square& NetworkGame::get(const Coordinates& c)
	{
		Coordinates chunk_coords = utils::to_chunk_coordinates(c);
		if (!chunk_requested(chunk_coords))
		{
			request_chunk(chunk_coords);
		}

		return board.get(c);
	}

	Square& NetworkGame::get(int x, int y)
	{
		return this->get({x, y});
	}

	void NetworkGame::send_clear_at(int x, int y)
	{
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.CLEAR_AT);

		auto clearAt = wrapper.mutable_clearat();
		clearAt->set_x(x);
		clearAt->set_y(y);

		client.send_message(wrapper);
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
			send_clear_at(x, y);
		}
		message::MessageWrapper wrapper;
		wrapper.set_type(wrapper.SQUARE_OPEN);

		auto squareOpen = wrapper.mutable_squareopen();
		squareOpen->set_x(x);
		squareOpen->set_y(y);

		client.send_message(wrapper);
	}
}
