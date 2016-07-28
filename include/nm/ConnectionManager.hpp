#ifndef NM_CONNECTION_HPP
#define NM_CONNECTION_HPP

#include <memory>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <netmine.pb.h>

namespace events = boost::signals2;

namespace nm
{
namespace server
{
	template<typename ToCast, typename FromCast>
	ToCast deref_cast(FromCast fromCast)
	{
		return *reinterpret_cast<ToCast*>(fromCast);
	}

	class Connection : public std::enable_shared_from_this<Connection>
	{
		public:
			typedef std::shared_ptr<Connection> ptr;

			static ptr create(boost::asio::io_service& io_service)
			{
				return ptr(new Connection(io_service));
			}

			boost::asio::ip::tcp::socket& socket()
			{
				return socket_;
			}

			void start();
			void sendMessage(message::MessageWrapper&);

			events::signal<void (ptr, nm::message::MessageWrapper&)> ev_message_received;

		private:
			void start_read();
			void connection_closed();
			void write_callback(const boost::system::error_code& ec, const size_t nbytes);
			void message_callback(uint32_t length, std::shared_ptr<uint8_t> data, const boost::system::error_code& ec, const size_t nbytes);
			void header_callback(std::shared_ptr<uint8_t>, const boost::system::error_code& ec, const size_t nbytes);
			boost::asio::ip::tcp::socket socket_;
			Connection(boost::asio::io_service& io_service);
	};

	class ConnectionManager
	{
		private:
			boost::asio::io_service io_service;
			boost::asio::ip::tcp::acceptor acceptor;

			std::vector<Connection::ptr> connections;
			void start_accept();
			void message_handler(Connection::ptr connection, nm::message::MessageWrapper& message);
			void handle_accept(Connection::ptr new_connection, const boost::system::error_code& error);


		public:
			events::signal<void (Connection::ptr, const message::SquareOpen&)> ev_square_open;
			events::signal<void (Connection::ptr, const message::SquareFlag&)> ev_square_flag;
			events::signal<void (Connection::ptr, const message::Player&)> ev_player_join;
			events::signal<void (Connection::ptr, const message::Player&)> ev_player_quit;
			events::signal<void (Connection::ptr, const message::CursorMove&)> ev_cursor_move;
			events::signal<void (Connection::ptr, const message::ChunkRequest&)> ev_chunk_request;

			ConnectionManager(boost::asio::ip::tcp::endpoint& endpoint);
			ConnectionManager(boost::asio::ip::tcp::endpoint endpoint);
			void send_all(message::MessageWrapper&);
			void send_all_other(Connection::ptr&, message::MessageWrapper&);
			void start();
	};
}
}

#endif // NM_CONNECTION_HPP
