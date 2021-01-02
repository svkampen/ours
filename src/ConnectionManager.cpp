#include <boost/log/trivial.hpp>
#include <cstdio>
#include <iostream>
#include <nm/ConnectionManager.hpp>

using boost::asio::ip::tcp;
namespace placeholders = boost::asio::placeholders;
using namespace std::placeholders;

namespace nm::server
{
    using error_code = boost::system::error_code;

    Connection::Connection(boost::asio::io_context& io_context): socket_(io_context) {};

    void Connection::start()
    {
        start_read();
    }

    void Connection::sendMessage(const message::MessageWrapper& wrapper)
    {
        size_t message_size = wrapper.ByteSizeLong();

        // Reserve 4 bytes for the length of the packet
        size_t total_size = message_size + 4;

        uint8_t buffer[total_size];
        wrapper.SerializeToArray(buffer + 4, message_size);

        uint32_t header = htonl(message_size);
        memcpy(buffer, &header, 4);

        boost::asio::async_write(socket_,
                                 boost::asio::buffer(buffer, total_size),
                                 boost::asio::transfer_all(),
                                 [](const error_code& ec, const size_t) {
                                     BOOST_LOG_TRIVIAL(debug)
                                         << "Write done with error code " << ec;
                                 });
    }

    void Connection::start_read()
    {
        std::shared_ptr<uint8_t[]> header_buf(new uint8_t[4]);

        boost::asio::async_read(
            socket_,
            boost::asio::buffer(header_buf.get(), 4),
            boost::asio::transfer_exactly(4),
            [=](const error_code& ec, const size_t sz) { header_callback(header_buf, ec, sz); });
    }

    void Connection::connection_closed()
    {
        BOOST_LOG_TRIVIAL(info) << "[net] Connection closed.";
        message::MessageWrapper wrapper;
        wrapper.set_type(message::MessageWrapper_Type_PLAYER_QUIT);
        this->ev_message_received(*this, wrapper);
    }

    void Connection::header_callback(std::shared_ptr<uint8_t[]> data, const error_code& ec,
                                     const size_t)
    {
        if (ec.value() != 0)
        {
            return this->connection_closed();
        }

        const uint8_t* bytes = data.get();
        uint32_t header;
        memcpy(&header, bytes, 4);

        uint32_t length = ntohl(header);

        std::shared_ptr<uint8_t[]> message_buf(new uint8_t[length]);

        boost::asio::async_read(socket_,
                                boost::asio::buffer(message_buf.get(), length),
                                boost::asio::transfer_exactly(length),
                                [=](const error_code& ec, const size_t sz) {
                                    message_callback(length, message_buf, ec, sz);
                                });
    }

    void Connection::message_callback(uint32_t length, std::shared_ptr<uint8_t[]> data,
                                      const error_code& ec, const size_t)
    {
        BOOST_LOG_TRIVIAL(debug) << "Read done with error code " << ec;
        message::MessageWrapper wrapper;
        wrapper.ParseFromArray(data.get(), length);

        this->ev_message_received(*this, wrapper);
        this->start_read();
    }

    ConnectionManager::ConnectionManager(tcp::endpoint& endpoint):
        acceptor(this->io_context, endpoint), accepting_connection(this->io_context) {};

    ConnectionManager::ConnectionManager(tcp::endpoint endpoint):
        acceptor(this->io_context, endpoint), accepting_connection(this->io_context) {};

    void ConnectionManager::start()
    {
        BOOST_LOG_TRIVIAL(info) << "[net] Starting server";
        start_accept();
        io_context.run();
    }

    void ConnectionManager::message_handler(Connection& connection,
                                            message::MessageWrapper& message)
    {
        event_map.fire(message.type(), connection, message);
    }

    void ConnectionManager::send_all(const message::MessageWrapper& wrapper)
    {
        for (auto& conn : this->connections)
        {
            conn.sendMessage(wrapper);
        }
    }

    void ConnectionManager::send_all_other(Connection& excluded,
                                           const message::MessageWrapper& wrapper)
    {
        for (auto& conn : this->connections)
        {
            if (conn != excluded)
            {
                conn.sendMessage(wrapper);
            }
        }
    }

    void ConnectionManager::start_accept()
    {
        accepting_connection =
            Connection(static_cast<boost::asio::io_context&>(acceptor.get_executor().context()));

        accepting_connection.ev_message_received.connect(
            [=](Connection& c, message::MessageWrapper& w) { message_handler(c, w); });

        acceptor.async_accept(accepting_connection.socket(),
                              [this](const error_code& ec) { this->handle_accept(ec); });
    }

    void ConnectionManager::handle_accept(const error_code& error)
    {
        if (!error)
        {
            Connection& c = connections.emplace_back(std::move(accepting_connection));
            auto ip_addr  = c.socket().remote_endpoint().address().to_string();
            c.start();
            BOOST_LOG_TRIVIAL(info) << "[net] Accepted new connection from " << ip_addr;
        }
        start_accept();
    }

    void ConnectionManager::disconnect(Connection& to_disconnect)
    {
        connections.erase(std::find(connections.cbegin(), connections.cend(), to_disconnect));
    }
}  // namespace nm::server
