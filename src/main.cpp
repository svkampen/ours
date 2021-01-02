#include <boost/log/trivial.hpp>
#include <ctime>
#include <cxxabi.h>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <nm/Argparse.hpp>
#include <nm/Client.hpp>
#include <nm/Config.hpp>
#include <nm/ConnectionManager.hpp>
#include <nm/CursesGui.hpp>
#include <nm/CursesSetupTeardown.hpp>
#include <nm/ImageSaver.hpp>
#include <nm/Logging.hpp>
#include <nm/NetworkGame.hpp>
#include <nm/SaveLoad.hpp>
#include <nm/Server.hpp>
#include <nm/Square.hpp>
#include <nm/Utils.hpp>
#include <sys/ioctl.h>
#include <typeinfo>
#include <unistd.h>

extern const char* const VERSION = "v0.1";

using boost::asio::ip::tcp;
using nlohmann::json;
using namespace std::literals;
using namespace std::placeholders;

int exit_event = eventfd(0, EFD_SEMAPHORE);

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

    gui.ev_square_open.connect(std::bind(&nm::NetworkGame::open_square_handler, &game, _1, _2));
    gui.ev_square_flag.connect(std::bind(&nm::NetworkGame::flag_square_handler, &game, _1, _2));
    gui.ev_cursor_move.connect(std::bind(&nm::NetworkGame::cursor_move_handler, &game, _1, _2));

    gui.ev_exit.connect([&io_service]() { io_service.stop(); });

    gui.ev_save_image.connect(std::bind(&nm::ImageSaver::save, &im, _1));

    void (nm::Gui::*nph_player)(const nm::message::Player&)         = &nm::Gui::new_player_handler;
    void (nm::Gui::*nph_mwrper)(const nm::message::MessageWrapper&) = &nm::Gui::new_player_handler;

    client.event_map.connect(MessageType(CHUNK_BYTES),
                             std::bind(&nm::NetworkGame::chunk_update_handler, &game, _1));

    client.event_map.connect(MessageType(CURSOR_MOVE),
                             std::bind(&nm::Gui::cursor_move_handler, &gui, _1));

    client.event_map.connect(MessageType(PLAYER_JOIN), std::bind(nph_mwrper, &gui, _1));

    client.event_map.connect(MessageType(PLAYER_QUIT),
                             std::bind(&nm::Gui::player_quit_handler, &gui, _1));

    game.ev_board_update.connect(std::bind(&nm::Gui::draw_board, &gui));
    game.ev_new_player.connect(std::bind(nph_player, &gui, _1));

    client.connect(nm::config["host"], nm::config["port"]);

    boost::asio::posix::stream_descriptor exit_event_desc(io_service, exit_event);
    exit_event_desc.async_read_some(boost::asio::null_buffers(),
                                    [](const boost::system::error_code&, const size_t) {
                                        throw nm::utils::exit_unwind_stack {};
                                    });

    gui.start();
}

void startServer()
{
    std::optional<nm::Game> maybeGame = nm::Loader::loadGame(nm::config["game"]);
    nm::Game game                     = maybeGame ? std::move(*maybeGame) : std::move(nm::Game());

    game.save_on_destruct();

    boost::asio::io_context ctx;
    boost::asio::posix::stream_descriptor exit_event_desc(ctx, exit_event);
    exit_event_desc.async_read_some(boost::asio::null_buffers(),
                                    [](const boost::system::error_code&, const size_t) {
                                        throw nm::utils::exit_unwind_stack {};
                                    });

    nm::server::Server server(ctx, game, std::stoi(nm::config["port"].get<std::string>()));
    server.start();
}

/* Signal to the event loop, by writing a byte
 * on the exit_event eventfd, that we should exit the game. */
void signal_exit()
{
    size_t val = 1;
    write(exit_event, &val, sizeof val);
}

int main(int argc, char* argv[])
{
    try
    {
        nlohmann::json default_arguments = {{"port", "4096"},
                                            {"save_path", "./save.nm"},
                                            {"game", "save.nm"},
                                            {"log_file", "ours.log"},
                                            {"show_overflagged", true}};
        nm::config.merge(default_arguments);

        nm::options_map arguments = nm::parse_options(argc, argv);

        if (!(nm::config.load(arguments["config_file"]) || nm::config.load(".nmrc") ||
              nm::config.load(getenv("HOME") + "/.nmrc"s) || nm::config.load("/etc/nmrc")))
        {
            if (!arguments["config_file"].empty())
            {
                const char* const err_message =
                    "[nm] Unable to load config file specified, are you sure you have the correct permissions, et cetera?";
                BOOST_LOG_TRIVIAL(error) << err_message;
                std::cerr << err_message << std::endl;
                return -1;
            }
        }

        nm::config.merge(arguments);

        signal(SIGINT, [](int) { signal_exit(); });
        signal(SIGTERM, [](int) { signal_exit(); });

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
    return 0;
}
