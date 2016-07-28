#include <nm/Client.hpp>

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
			exit(1);
		}
	}

	void Client::handle_connect(const boost::system::error_code &ec, tcp::resolver::iterator ep_iter)
	{
		if (!ec)
		{
			this->ev_connected();
			start_read();
		} else
		{
			// Let's try the next endpoint.
			socket_.close();
			auto endpoint = *ep_iter;
			socket_.async_connect(endpoint,
					boost::bind(&Client::handle_connect, this, _1, ++ep_iter));
		}
	}

	void Client::start_read()
	{
		std::shared_ptr<uint8_t> header_buf(new uint8_t[4], std::default_delete<uint8_t[]>());

		boost::asio::async_read(
				socket_,
				boost::asio::buffer(header_buf.get(), 4),
				boost::asio::transfer_at_least(4),
				boost::bind(
					&Client::header_callback,
					this,
					header_buf,
					_1,
					_2));
	}

	void Client::header_callback(std::shared_ptr<uint8_t> data, const boost::system::error_code &ec, const size_t nbytes)
	{
		if (ec != 0)
		{
			exit(1);
		}

		const uint8_t* bytes = data.get();
		uint32_t header;
		memcpy(&header, bytes, 4);

		uint32_t length = ntohl(header);

		std::shared_ptr<uint8_t> message_buf(new uint8_t[length], std::default_delete<uint8_t[]>());

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
		message::MessageWrapper wrapper;
		wrapper.ParseFromArray(data.get(), length);

		BOOST_LOG_TRIVIAL(info) << "Message received: " << wrapper.DebugString();

		switch (wrapper.type())
		{
			case wrapper.WELCOME:
				this->ev_welcome(wrapper.welcome());
				break;
			case wrapper.CHUNK_BYTES:
				this->ev_update_chunk(wrapper.chunkbytes());
				break;
			case wrapper.PLAYER_JOIN:
				this->ev_player_join(wrapper.playerjoin());
				break;
			case wrapper.PLAYER_QUIT:
				this->ev_player_quit(wrapper.playerquit());
				break;
			case wrapper.CURSOR_MOVE:
				this->ev_cursor_move(wrapper.cursormove());
				break;
			default:
				break;
		}

		start_read();
	}

	void Client::send_message(const message::MessageWrapper& wrapper)
	{
		BOOST_LOG_TRIVIAL(info) << "Sending message: " << wrapper.DebugString();
		size_t message_size = wrapper.ByteSize();

		// Reserve 4 bytes for the header
		size_t total_size = message_size + 4;

		char* buffer = (char*)malloc(total_size);
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
	};

	void Client::poll()
	{
		io_service.poll();
	}
}
