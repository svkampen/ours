#include <nm/Utils.hpp>
#include <nm/Config.hpp>
#include <nm/Gui.hpp>
#include <nm/Client.hpp>
#include <nm/NetworkGame.hpp>
#include <nm/Square.hpp>
#include <nm/ConnectionManager.hpp>
#include <nm/Argparse.hpp>
#include <nm/Logging.hpp>
#include <nm/Server.hpp>
#include <nm/SaveLoad.hpp>
#include <nm/CursesSetupTeardown.hpp>
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

#include <readline/readline.h>
#include <readline/history.h>

#include <ctime>
#include <typeinfo>
#include <sys/ioctl.h>

#include <fstream>
#include <json.hpp>
#include <cxxabi.h>

extern const char* const VERSION = "v0.1";
nm::Game *GLOBAL_GAME = nullptr;
nm::Gui *GLOBAL_GUI = nullptr;

using boost::asio::ip::tcp;
using nlohmann::json;
using namespace std::literals;

void rl_callback_handler(char *line)
{
	add_history(line);
	std::string command = std::string(line);
	free(line);

	GLOBAL_GUI->handle_command_input(command);
}

int hook_input_avail()
{
	struct timeval tv = {0L, 0L};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);

	return select(1, &fds, NULL, NULL, &tv);
}

void hook_redisplay()
{
	GLOBAL_GUI->command << nm::Erase << ":";

	int input_prompt_len = 1;

	int cur_point = input_prompt_len + rl_point;

	GLOBAL_GUI->command << rl_line_buffer << nm::ClrToBot;
	GLOBAL_GUI->command << nm::Move({cur_point % COLS, 0}) << nm::Refresh;
}

void startClient()
{
	nm::CursesSetupTeardown cst;

	boost::asio::io_service io_service;
	nm::Client client(io_service);
	nm::NetworkGame game(client);
	nm::Gui gui(io_service, game);
	GLOBAL_GUI = &gui;

	gui.ev_square_open.connect(boost::bind(&nm::NetworkGame::open_square_handler, &game, _1, _2));
	gui.ev_square_flag.connect(boost::bind(&nm::NetworkGame::flag_square_handler, &game, _1, _2));
	gui.ev_cursor_move.connect(boost::bind(&nm::NetworkGame::cursor_move_handler, &game, _1, _2));

	gui.ev_exit.connect([&io_service](){ io_service.stop(); });

	client.ev_update_chunk.connect(boost::bind(&nm::NetworkGame::chunk_update_handler, &game, _1));
	client.ev_cursor_move.connect(boost::bind(&nm::Gui::cursor_move_handler, &gui, _1));
	client.ev_player_join.connect(boost::bind(&nm::Gui::new_player_handler, &gui, _1));
	client.ev_player_quit.connect(boost::bind(&nm::Gui::player_quit_handler, &gui, _1));

	game.ev_board_update.connect(boost::bind(&nm::Gui::draw_board, &gui));
	game.ev_new_player.connect(boost::bind(&nm::Gui::new_player_handler, &gui, _1));

	client.connect(nm::config["host"], nm::config["port"]);

	io_service.run();
}

void startServer()
{
	std::optional<nm::Game> maybeGame = nm::Loader::loadGame(nm::config["game"]);
	nm::Game game = maybeGame.has_value() ? std::move(*maybeGame) : std::move(nm::Game());

	game.save_on_destruct();

	GLOBAL_GAME = &game;
	nm::server::Server server(game, std::stoi(nm::config["port"].get<std::string>()));
	server.start();
}

std::string demangle(const char* mangled)
{
#ifdef __GNUG__
	int status = -1;
	char *demangled = nullptr;

	demangled = abi::__cxa_demangle(mangled, 0, 0, &status);
	return std::string(demangled);
#else
	return std::string(mangled);
#endif
}

int main(int argc, char *argv[])
{
	try
	{
		nm::options_map arguments = nm::parse_options(argc, argv);
		nm::config.merge(arguments);

		if (!(nm::config.load(nm::config["config_file"]) || nm::config.load(".nmrc") || nm::config.load(getenv("HOME") + "/.nmrc"s) || nm::config.load("/etc/nmrc")))
		{
			const char* const err_message = "[nm] Unable to load config file specified, are you sure you have the correct permissions, et cetera?";
			BOOST_LOG_TRIVIAL(error) << err_message;
			std::cerr << err_message << std::endl;
			return -1;
		}

		signal(SIGINT, [](int _sig){throw nm::utils::exit_unwind_stack(); });

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

	} catch (nm::utils::exit_unwind_stack &e) {
	} catch (std::exception &e)
	{
		const char* exception_name_mangled = typeid(e).name();
		std::string exception_name = demangle(exception_name_mangled);
		std::cerr << "Received error: " << exception_name << "(what: " << e.what() << ")." << std::endl;
	}
	return 0;
}
