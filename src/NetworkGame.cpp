#include <algorithm>
#include <boost/log/trivial.hpp>
#include <ours.pb.h>
#include <nm/Config.hpp>
#include <nm/ImageSaver.hpp>
#include <nm/NetworkGame.hpp>
#include <nm/Utils.hpp>
#include <functional>

using namespace std::placeholders;

namespace nm
{
    NetworkGame::NetworkGame(Client& client): client(client), board(ChunkGenerator(0.0, 0.0))
    {
        board.set_client_mode(true);
        is_first_open.set();

        client.ev_connected.connect(boost::bind(&NetworkGame::connected_handler, this));

        client.event_map.connect(message::MessageWrapper_Type_WELCOME,
                                 std::bind(&NetworkGame::welcome_handler, this, _1));
    }

    void NetworkGame::connected_handler()
    {
        connected = true;
        this->send_player_join(0, 0);
        this->ev_board_update();
    }

    void NetworkGame::chunk_update_handler(const message::MessageWrapper& mwpr)
    {
        auto msg   = mwpr.chunkbytes();
        char* data = const_cast<char*>(msg.data().c_str());

        Chunk chunk;
        chunk.deserialize(data);

        Coordinates coordinates = {msg.x(), msg.y()};

        board.add_chunk(coordinates, chunk);

        if (!chunk_requested(coordinates))
            requested_chunks.push_back(coordinates);

        this->ev_board_update();
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

    void NetworkGame::welcome_handler(const message::MessageWrapper& mwpr)
    {
        auto welcome = mwpr.welcome();
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
        return std::find(requested_chunks.begin(), requested_chunks.end(), c) !=
               requested_chunks.end();
    }

    inline Square& NetworkGame::get(const Coordinates& c)
    {
        return board.get(c);
    }

    inline Square& NetworkGame::get(int x, int y)
    {
        return this->get({x, y});
    }

    inline const Square& NetworkGame::get(const Coordinates& c) const
    {
        return board.get(c);
    }

    inline const Square& NetworkGame::get(int x, int y) const
    {
        return this->get({x, y});
    }

    const ChunkList& NetworkGame::get_chunks() const
    {
        return board.get_chunks();
    }

    std::optional<Chunk* const> NetworkGame::get_chunk(const Coordinates& coordinates)
    {
        return board.get_chunk(coordinates);
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
        auto playerJoin = wrapper.mutable_player();
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
}  // namespace nm
