#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <nm/Argparse.hpp>
#include <nm/Client.hpp>
#include <nm/Config.hpp>
#include <nm/CursesGui.hpp>
#include <nm/CursesSetupTeardown.hpp>
#include <nm/ImageSaver.hpp>
#include <nm/Logging.hpp>
#include <nm/NetworkGame.hpp>
#include <nm/SaveLoad.hpp>
#include <nm/Server.hpp>
#include <nm/Utils.hpp>
#include <sys/ioctl.h>
#include <typeinfo>

extern const char* const VERSION = "v0.1";

using boost::asio::ip::tcp;
using nlohmann::json;
using namespace std::literals;

void startClient()
{
	if (!nm::config.contains("host"))
	{
		std::cerr << "Client requested, but no host passed. Add --host <host> "
					 "to your command line?"
				  << std::endl;
		return;
	}
	nm::CursesSetupTeardown cst;

	boost::asio::io_service io_service;
	nm::Client client(io_service);
	nm::NetworkGame game(client);
	nm::ImageSaver im(game);
	nm::curses::CursesGui gui(io_service, game);

	gui.ev_square_open.connect(boost::bind(&nm::NetworkGame::open_square_handler, &game, _1, _2));
	gui.ev_square_flag.connect(boost::bind(&nm::NetworkGame::flag_square_handler, &game, _1, _2));
	gui.ev_cursor_move.connect(boost::bind(&nm::NetworkGame::cursor_move_handler, &game, _1, _2));

	gui.ev_exit.connect([&io_service](){ io_service.stop(); });

	gui.ev_save_image.connect(boost::bind(&nm::ImageSaver::save, &im, _1));

    void (nm::Gui::*nph_player) (const nm::message::Player&) = &nm::Gui::new_player_handler;
    void (nm::Gui::*nph_mwrper) (const nm::message::MessageWrapper&) = &nm::Gui::new_player_handler;

	client.event_map.connect(MessageType(CHUNK_BYTES),
		boost::bind(&nm::NetworkGame::chunk_update_handler, &game, _1));

	client.event_map.connect(MessageType(CURSOR_MOVE),
		boost::bind(&nm::Gui::cursor_move_handler, &gui, _1));

	client.event_map.connect(MessageType(PLAYER_JOIN),
        boost::bind(nph_mwrper, &gui, _1));

	client.event_map.connect(MessageType(PLAYER_QUIT),
		boost::bind(&nm::Gui::player_quit_handler, &gui, _1));

	game.ev_board_update.connect(boost::bind(&nm::Gui::draw_board, &gui));
    game.ev_new_player.connect(boost::bind(nph_player, &gui, _1));

	client.connect(nm::config["host"], nm::config["port"]);

    gui.start();
}

void startServer()
{
	std::optional<nm::Game> maybeGame = nm::Loader::loadGame(nm::config["game"]);
	nm::Game game = maybeGame ? std::move(*maybeGame) : std::move(nm::Game());

	game.save_on_destruct();

	nm::server::Server server(game, std::stoi(nm::config["port"].get<std::string>()));
	server.start();
}

int main(int argc, char *argv[])
{
	try
	{
		nlohmann::json default_arguments = {
			{"port", "4096"},
			{"save_path", "./save.nm"},
			{"game", "save.nm"},
			{"log_file", "nm.log"},
			{"show_overflagged", true}
		};
		nm::config.merge(default_arguments);

		nm::options_map arguments = nm::parse_options(argc, argv);

		if (!(nm::config.load(arguments["config_file"]) || nm::config.load(".nmrc") || nm::config.load(getenv("HOME") + "/.nmrc"s) || nm::config.load("/etc/nmrc")))
		{
			if (!arguments["config_file"].empty())
			{
				const char* const err_message = "[nm] Unable to load config file specified, are you sure you have the correct permissions, et cetera?";
				BOOST_LOG_TRIVIAL(error) << err_message;
				std::cerr << err_message << std::endl;
				return -1;
			}
		}

		nm::config.merge(arguments);

		signal(SIGINT, [](int _sig) { throw nm::utils::exit_unwind_stack(); });
		signal(SIGTERM, [](int _sig) { throw nm::utils::exit_unwind_stack(); });

		if (nm::config["server"] == "true")
		{
			logging_init(nm::config["log_file"], true);
			startServer();
		}
		else
		{
			logging_init(nm::config["log_file"]);
			startClient();
		}
	}
	catch (nm::utils::exit_unwind_stack& e)
	{
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception received, exiting netmine..." << std::endl
				  << e.what() << std::endl;
	}
	return 0;
}
