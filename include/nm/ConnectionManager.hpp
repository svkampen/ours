#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <ours.pb.h>
#include <nm/EventMap.hpp>

namespace events = boost::signals2;

namespace nm::server
{
    class Connection
    {
      public:
        [[nodiscard]] const boost::asio::ip::tcp::socket& socket() const
        {
            return socket_;
        }

        boost::asio::ip::tcp::socket& socket()
        {
            return socket_;
        }

        bool operator==(const Connection& other) const
        {
            return (other.socket_.is_open() == this->socket_.is_open()) &&
                   (other.socket_.remote_endpoint() == this->socket_.remote_endpoint());
        }

        bool operator!=(const Connection& other) const
        {
            return !(other == *this);
        }

        explicit Connection(boost::asio::io_context& io_context);

        void start();
        void sendMessage(const message::MessageWrapper&);

        events::signal<void(Connection&, nm::message::MessageWrapper&)> ev_message_received;

      private:
        void start_read();
        void connection_closed();

        void message_callback(uint32_t length, std::shared_ptr<uint8_t[]> data,
                              const boost::system::error_code& ec, size_t nbytes);

        void header_callback(std::shared_ptr<uint8_t[]> data, const boost::system::error_code& ec,
                             size_t nbytes);

        boost::asio::ip::tcp::socket socket_;
    };

    class ConnectionManager
    {
      private:
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::acceptor acceptor;

        Connection accepting_connection;
        std::deque<Connection> connections;

        void start_accept();
        void handle_accept(const boost::system::error_code& error);

        void message_handler(Connection& connection, nm::message::MessageWrapper& message);

      public:
        nm::EventMap<message::MessageWrapper_Type, Connection&, const message::MessageWrapper&>
            event_map;

        explicit ConnectionManager(const boost::asio::ip::tcp::endpoint& endpoint);

        void send_all(const message::MessageWrapper&);
        void send_all_other(Connection&, const message::MessageWrapper&);
        void start();
        void disconnect(Connection& to_disconnect);
    };
}  // namespace nm::server
