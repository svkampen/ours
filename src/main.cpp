#include "Board.hpp"
#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

#include <fstream>

using boost::asio::ip::tcp;

void logging_init() {
	boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend =
		boost::make_shared<boost::log::sinks::text_ostream_backend>();
	backend->add_stream(boost::shared_ptr<std::ostream>(new std::ofstream("netmine.log")));

	backend->auto_flush(true);

	typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > sink_t;
	boost::shared_ptr< sink_t > sink(new sink_t(backend));

	boost::log::core::get()->add_sink(sink);

	boost::log::add_common_attributes();
	boost::log::core::get()->set_filter(
			boost::log::trivial::severity >= boost::log::trivial::info
	);
}
int main(int argc, char *argv[]) {
    nm::Board board;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            nm::Square& square = board.get({i, j});
            if (square.is_mine)
                std::cout << " * ";
            else
                std::cout << "   ";
        }
        std::cout << std::endl;
    }
}
