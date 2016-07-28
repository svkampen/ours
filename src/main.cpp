#include <nm/Gui.hpp>
#include <nm/Client.hpp>
#include <nm/NetworkGame.hpp>
#include <nm/Square.hpp>
#include <nm/ConnectionManager.hpp>
#include <nm/Argparse.hpp>
#include <nm/Logging.hpp>
#include <nm/Server.hpp>
#include <nm/SaveLoad.hpp>
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
#include <json.hpp>

extern const char* const VERSION = "v0.1";
nm::Game *default_game = nullptr;

using boost::asio::ip::tcp;
using nlohmann::json;

void nm_exit(int x)
{
	BOOST_LOG_TRIVIAL(info) << "";
	BOOST_LOG_TRIVIAL(info) << "SIGINT received, saving game to ./save.nm";
	if (default_game != nullptr)
	{
		nm::Saver::saveGame(*default_game, "./save.nm");
	}
	endwin();
	exit(0);
}


void nm_exit()
{
	nm_exit(0);
}

json merge_json(const json& a, const json& b)
{
	json result = a.flatten();
	json tmp = b.flatten();

	for (auto it = tmp.begin(); it != tmp.end(); ++it)
	{
		if (result.find(it.key()) == result.end())
			result[it.key()] = it.value();
	}

	return result.unflatten();
}

void startClient(json config)
{
	nm::init_curses();

	boost::asio::io_service io_service;
	nm::Client client(io_service);
	nm::NetworkGame game(client);
	nm::Gui gui(io_service, game);

	gui.ev_square_open.connect(boost::bind(&nm::NetworkGame::open_square_handler, &game, _1, _2));
	gui.ev_square_flag.connect(boost::bind(&nm::NetworkGame::flag_square_handler, &game, _1, _2));
	gui.ev_cursor_move.connect(boost::bind(&nm::NetworkGame::cursor_move_handler, &game, _1, _2));

	gui.ev_exit.connect((void(*)())nm_exit);
	gui.ev_save_image.connect(boost::bind(&nm::NetworkGame::save_image_handler, &game));

	client.ev_update_chunk.connect(boost::bind(&nm::NetworkGame::chunk_update_handler, &game, _1));
	client.ev_cursor_move.connect(boost::bind(&nm::Gui::cursor_move_handler, &gui, _1));
	client.ev_player_join.connect(boost::bind(&nm::Gui::new_player_handler, &gui, _1));
	client.ev_player_quit.connect(boost::bind(&nm::Gui::player_quit_handler, &gui, _1));

	game.ev_board_update.connect(boost::bind(&nm::Gui::draw_board, &gui));
	game.ev_new_player.connect(boost::bind(&nm::Gui::new_player_handler, &gui, _1));

	client.connect("segfault.party", "4096");

	while (true) {
		io_service.run();
	}
}

void startServer(json config)
{
	nm::Game game;
	if (config["game"] != "")
	{
		boost::optional<nm::Game> maybeGame = nm::Loader::loadGame(config["game"]);

		if (maybeGame)
		{
			game = *maybeGame;
		}
		else
		{
			BOOST_LOG_TRIVIAL(warning) << "[nm] Unable to load saved game, starting a new one!";
		}
	}
	default_game = &game;
	nm::server::Server server(game, std::stoi(config["port"].get<std::string>()));
	server.start();
}

int main(int argc, char *argv[])
{
	nm::options_map arguments = nm::parse_options(argc, argv);

	json config(arguments);

	std::ifstream config_file;
	config_file.open(config["config_file"]);

	if (config_file.is_open())
		config = merge_json(config, json(config_file));


	signal(SIGINT, nm_exit);

	if (config["server"] == "true")
	{
		logging_init(config["log_file"], true);
		startServer(config);
	}
	else
	{
		logging_init(config["log_file"]);
		startClient(config);
	}

	return 0;
}
