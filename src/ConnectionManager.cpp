#include <nm/ConnectionManager.hpp>
#include <iostream>
#include <cstdio>

#include <boost/log/trivial.hpp>
#include <boost/bind.hpp>

#include <nm/netmine.pb.h>

using boost::asio::ip::tcp;
namespace placeholders = boost::asio::placeholders;

namespace nm
{
	namespace server
	{
		Connection::Connection(boost::asio::io_service& io_service) : socket_(io_service)
		{
		};

		void Connection::start()
		{
			start_read();
		}

		void Connection::sendMessage(message::MessageWrapper& wrapper)
		{
			size_t message_size = wrapper.ByteSize();

			// Reserve 4 bytes for the length of the packet
			size_t total_size = message_size + 4;

			uint8_t buffer[total_size];
			wrapper.SerializeToArray(buffer + 4, message_size);

			uint32_t header = htonl(message_size);
			memcpy(buffer, &header, 4);

			boost::asio::async_write(
					socket_,
					boost::asio::buffer(buffer, total_size),
					boost::asio::transfer_all(),
					boost::bind(
						&Connection::write_callback,
						shared_from_this(),
						placeholders::error,
						placeholders::bytes_transferred));
		}

		void Connection::start_read()
		{
			std::shared_ptr<uint8_t> header_buf(new uint8_t[4], std::default_delete<uint8_t[]>());

			boost::asio::async_read(
					socket_,
					boost::asio::buffer(header_buf.get(), 4),
					boost::asio::transfer_at_least(4),
					boost::bind(
						&Connection::header_callback,
						shared_from_this(),
						header_buf,
						placeholders::error,
						placeholders::bytes_transferred));
		}

		void Connection::write_callback(const boost::system::error_code& ec, const size_t nbytes)
		{
		};

		void Connection::connection_closed()
		{
			BOOST_LOG_TRIVIAL(info) << "[net] Connection from " << socket_.remote_endpoint().address().to_string() << " closed.";
			message::MessageWrapper wrapper;
			wrapper.set_type(message::MessageWrapper_Type_PLAYER_QUIT);
			this->ev_message_received(shared_from_this(), wrapper);
		}

		void Connection::header_callback(std::shared_ptr<uint8_t> data, const boost::system::error_code& ec, const size_t nbytes)
		{
			if (ec != 0)
			{
				return this->connection_closed();
			}

			int length = deref_cast<int>(data.get());

			BOOST_LOG_TRIVIAL(info) << "[net] read header: length " << length;

			std::shared_ptr<uint8_t> message_buf(new uint8_t[length], std::default_delete<uint8_t[]>());

			boost::asio::async_read(
					socket_,
					boost::asio::buffer(message_buf.get(), length),
					boost::asio::transfer_at_least(length),
					boost::bind(
						&Connection::message_callback,
						shared_from_this(),
						length,
						message_buf,
						placeholders::error,
						placeholders::bytes_transferred));
		}

		void Connection::message_callback(uint32_t length, std::shared_ptr<uint8_t> data, const boost::system::error_code& ec, const size_t nbytes)
		{
			message::MessageWrapper wrapper;
			wrapper.ParseFromArray(data.get(), length);

			this->ev_message_received(shared_from_this(), wrapper);
			this->start_read();
		}

		ConnectionManager::ConnectionManager(tcp::endpoint& endpoint) : acceptor(io_service, endpoint)
		{
		};

		ConnectionManager::ConnectionManager(tcp::endpoint endpoint) : acceptor(io_service, endpoint)
		{
		};

		void ConnectionManager::start()
		{
			BOOST_LOG_TRIVIAL(info) << "[net] Starting server";
			start_accept();
		}

		void ConnectionManager::poll()
		{
			io_service.poll();
		}

		void ConnectionManager::message_handler(Connection::ptr connection, message::MessageWrapper& message)
		{
			switch (message.type())
			{
				case message::MessageWrapper_Type_SQUARE_OPEN:
					this->ev_square_open(connection, message.squareopen());
					break;
				case message::MessageWrapper_Type_SQUARE_FLAG:
					this->ev_square_flag(connection, message.squareflag());
					break;
				case message::MessageWrapper_Type_CURSOR_MOVE:
					this->ev_cursor_move(connection, message.cursormove());
					break;
				case message::MessageWrapper_Type_PLAYER_JOIN:
					this->ev_player_join(connection, message.playerjoin());
					break;
				case message::MessageWrapper_Type_PLAYER_QUIT:
					this->ev_player_quit(connection, message.playerquit());
					break;
				default:
					BOOST_LOG_TRIVIAL(warning) << "[net] Unhandled message type arrived: " << static_cast<uint16_t>(message.type());
					break;
			}
		}

		void ConnectionManager::send_all(message::MessageWrapper& wrapper)
		{
			for(auto&& conn : this->connections)
			{
				conn->sendMessage(wrapper);
			}
		}

		void ConnectionManager::send_all_other(Connection::ptr& exclusion, message::MessageWrapper& wrapper)
		{
			for(auto&& conn : this->connections)
			{
				if (conn != exclusion)
				{
					conn->sendMessage(wrapper);
				}
			}
		}

		void ConnectionManager::start_accept()
		{
			Connection::ptr new_connection = Connection::create(acceptor.get_io_service());
			connections.push_back(new_connection);
			new_connection->ev_message_received.connect(boost::bind(
					&ConnectionManager::message_handler,
					this,
					_1,
					_2));

			acceptor.async_accept(new_connection->socket(),
				boost::bind(&ConnectionManager::handle_accept, this, new_connection, placeholders::error));
		}

		void ConnectionManager::handle_accept(Connection::ptr new_connection, const boost::system::error_code& error)
		{
			if (!error)
			{
				std::string ip_addr = new_connection->socket().remote_endpoint().address().to_string();
				BOOST_LOG_TRIVIAL(info) << "[net] Accepted new connection from " << ip_addr;
				new_connection->start();
			}

			start_accept();
		}
	}
}
