#include <nm/Gui.hpp>
#include <nm/Client.hpp>
#include <nm/NetworkGame.hpp>
#include <nm/Square.hpp>
#include <nm/ConnectionManager.hpp>
#include <nm/Server.hpp>
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

void nm_exit(int x)
{
	endwin();
	exit(0);
}

int main(int argc, char *argv[])
{
	logging_init();
	signal(SIGINT, nm_exit);
	if (strcmp("-s", argv[1]) == 0)
	{
		nm::server::Server server;
		server.start();
	} else
	{
		boost::asio::io_service io_service;
		nm::Client client(io_service, "localhost", "4096");
		nm::NetworkGame game(client);
		nm::Gui gui(io_service, game);

		gui.ev_square_open.connect(boost::bind(&nm::NetworkGame::open_square_handler, &game, _1, _2));
		gui.ev_square_flag.connect(boost::bind(&nm::NetworkGame::flag_square_handler, &game, _1, _2));

		client.ev_update_chunk.connect(boost::bind(&nm::NetworkGame::chunk_update_handler, &game, _1, _2));

		while (true) {
			io_service.run();
		}
	}
	/*
	nm::Gui gui;
	nm::Game game;

	gui.ev_square_open.connect(boost::bind(&nm::Game::open_square_handler, &game, _1, _2));
	gui.ev_square_flag.connect(boost::bind(&nm::Game::flag_square_handler, &game, _1, _2));
	gui.ev_exit.connect(&nm_exit);

	while (true) {
		gui.poll(game.board);
	}
	*/
}
