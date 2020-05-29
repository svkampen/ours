#ifndef NM_CLIENT_HPP
#define NM_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/signals2.hpp>
#include <netmine.pb.h>
#include <nm/EventMap.hpp>
#include <string>

namespace events = boost::signals2;

namespace nm
{
    class Client
    {
      private:
        std::string port;
        std::string ip;
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::socket socket_;
        boost::asio::io_service& io_service;

        void parse_message(char* message, int length);
        void start_read();
        void handle_resolve(const boost::system::error_code& ec,
                            boost::asio::ip::tcp::resolver::iterator ep_iter);
        void handle_connect(const boost::system::error_code& ec,
                            boost::asio::ip::tcp::resolver::iterator ep_iter);
        void header_callback(std::shared_ptr<uint8_t> data, const boost::system::error_code& ec,
                             const size_t nbytes);
        void message_callback(uint32_t length, std::shared_ptr<uint8_t> data,
                              const boost::system::error_code& ec, const size_t nbytes);
        void write_callback(const boost::system::error_code& ec, const size_t nbytes);

      public:
        Client(boost::asio::io_service& io_service);

        void send_message(const message::MessageWrapper& wrapper);
        void connect(const std::string& ip, const std::string& port);

        nm::EventMap<message::MessageWrapper_Type, const message::MessageWrapper&> event_map;
        events::signal<void()> ev_connected;
    };
}  // namespace nm

#endif  // NM_CLIENT_HPP
