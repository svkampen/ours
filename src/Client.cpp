#include <nm/Utils.hpp>
#include <nm/Client.hpp>
#include <nm/Config.hpp>
#include <cassert>

#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;

namespace nm
{
	Client::Client(boost::asio::io_service& io_service)
		: resolver_(io_service), socket_(io_service), io_service(io_service)
	{
	}

	void Client::connect(const std::string& ip, const std::string& port)
	{
		tcp::resolver::query query(ip, port);
		resolver_.async_resolve(query,
				boost::bind(&Client::handle_resolve, this, _1, _2));
	}

	void Client::handle_resolve(const boost::system::error_code &ec, tcp::resolver::iterator ep_iter)
	{
		if (!ec)
		{
			auto endpoint = *ep_iter;
			socket_.async_connect(endpoint,
					boost::bind(&Client::handle_connect, this, _1, ++ep_iter));
		} else
		{
			BOOST_LOG_TRIVIAL(info) << "Error: " << ec.message();
			throw std::runtime_error(ec.message());
		}
	}

	void Client::handle_connect(const boost::system::error_code &ec, tcp::resolver::iterator ep_iter)
	{
		static tcp::resolver::iterator end;
		if (!ec)
		{
			auto nsock = socket_.native_handle();

			/* Set TCP keepalive packets. */
			int yes = 1;
			int idle = 1;
			int count = 5;
			int interval = 30;
			int err = 0;
			socklen_t optlen = sizeof(idle);
			if (setsockopt(nsock, SOL_SOCKET, SO_KEEPALIVE, &yes, optlen) < 0)
			{
				err = 1;
				goto fail;
			}

			if (setsockopt(nsock, SOL_TCP, TCP_KEEPIDLE, &idle, optlen) < 0)
			{
				err = 2;
				goto fail;
			}

			if (setsockopt(nsock, SOL_TCP, TCP_KEEPINTVL, &interval, optlen) < 0)
			{
				err = 3;
				goto fail;
			}

			if (setsockopt(nsock, SOL_TCP, TCP_KEEPCNT, &count, optlen) < 0)
			{
				err = 4;
				goto fail;
			}


			this->ev_connected();
			start_read();

			return;

fail:
			BOOST_LOG_TRIVIAL(info) << "Failed to turn on keepalive (" << err << ")";

			this->ev_connected();
			start_read();
		} else
		{
			// Let's try the next endpoint.
			socket_.close();
			if (ep_iter != end)
			{
				auto endpoint = *ep_iter;
				socket_.async_connect(endpoint,
						boost::bind(&Client::handle_connect, this, _1, ++ep_iter));
			} else {
				std::stringstream ss;
				ss << "Unable to connect to specified host: " << nm::config["host"];
				throw std::runtime_error(ss.str());
			}
		}
	}

	void Client::start_read()
	{
		std::shared_ptr<uint8_t> header_buf(new uint8_t[4], std::default_delete<uint8_t[]>());

		BOOST_LOG_TRIVIAL(info) << "Waiting for packet...";
		boost::asio::async_read(
				socket_,
				boost::asio::buffer(header_buf.get(), 4),
				boost::asio::transfer_exactly(4),
				boost::bind(
					&Client::header_callback,
					this,
					header_buf,
					_1,
					_2));
	}

	void Client::header_callback(std::shared_ptr<uint8_t> data, const boost::system::error_code &ec, const size_t nbytes)
	{
		if (ec.value() != 0)
		{
            BOOST_LOG_TRIVIAL(error) << "ERROR! Got code " << ec << "when reading header.";
			throw std::runtime_error("Server no longer responding");
		}

		const uint8_t* bytes = data.get();
		uint32_t header;
		memcpy(&header, bytes, 4);

		uint32_t length = ntohl(header);

		std::shared_ptr<uint8_t> message_buf(new uint8_t[length], std::default_delete<uint8_t[]>());

        assert(length < 512);
		BOOST_LOG_TRIVIAL(info) << "Header read, reading rest of package (length " << length << ")";

		boost::asio::async_read(
				socket_,
				boost::asio::buffer(message_buf.get(), length),
				boost::asio::transfer_exactly(length),
				boost::bind(
					&Client::message_callback,
					this,
					length,
					message_buf,
					_1, _2));
	}

	void Client::message_callback(uint32_t length, std::shared_ptr<uint8_t> data, const boost::system::error_code& ec, const size_t nbytes)
	{
        if (ec.value() != 0)
        {
            BOOST_LOG_TRIVIAL(error) << "ERROR! Got code " << ec << "when reading message.";
        }
		BOOST_LOG_TRIVIAL(info) << "Message received.";

		message::MessageWrapper wrapper;
		wrapper.ParseFromArray(data.get(), length);

		event_map.fire(wrapper.type(), wrapper);
		start_read();
	}

	void Client::send_message(const message::MessageWrapper& wrapper)
	{
		BOOST_LOG_TRIVIAL(info) << "Sending message. ";
		size_t message_size = wrapper.ByteSizeLong();

		// Reserve 4 bytes for the header
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
					&Client::write_callback,
					this,
					_1, _2));
	}

	void Client::write_callback(const boost::system::error_code &ec, const size_t nbytes)
	{
		BOOST_LOG_TRIVIAL(info) << "Write done with error code " << ec;
	};

}
