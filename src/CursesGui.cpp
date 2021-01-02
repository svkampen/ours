#include <boost/log/trivial.hpp>
#include <nm/CursesGui.hpp>
#include <nm/Flag.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Utils.hpp>
#include <signal.h>
#include <sys/ioctl.h>

namespace nm::curses
{
    CursesGui::CursesGui(boost::asio::io_service& io_service, ChunkSquareSource& squareSource):
        Gui(squareSource),
        main(0, 0, COLS - 21, LINES - 1),
        sidebar(COLS - 20, 0, 20, LINES - 1),
        command(0, LINES - 1, COLS, 1),
        in(io_service, ::dup(STDIN_FILENO)),
        io_service(io_service),
        boardview(self_cursor, main, sidebar, ev_square_open, ev_square_flag, ev_cursor_move),
        chunkview(self_cursor, main, sidebar),
        current_view(&boardview)
    {
        main.nowrap = true;
        width = height = 0;
    }

    void CursesGui::player_quit_handler(const message::MessageWrapper& mwpr)
    {
        auto player = mwpr.playerquit();
        BOOST_LOG_TRIVIAL(info) << "Removing player with ID " << player.id();
        cursors.erase(player.id());
        draw_board();
        this->maybe_draw_sidebar();
    }

    void CursesGui::new_player_handler(const message::Player& player)
    {
        BOOST_LOG_TRIVIAL(info) << "Adding cursor for new player with ID " << player.id();
        cursors[player.id()] = {.x        = player.x(),
                                .y        = player.y(),
                                .offset_x = 0,
                                .offset_y = 0,
                                .color    = rand() % 4 + 2};
        draw_board();
        this->maybe_draw_sidebar();
    }

    void CursesGui::cursor_move_handler(const message::MessageWrapper& mwpr)
    {
        auto msg = mwpr.cursormove();
        BOOST_LOG_TRIVIAL(info) << "Moving cursor for " << msg.id();
        auto&& data = cursors[msg.id()];
        data.x      = msg.x();
        data.y      = msg.y();
        draw_board();
        this->maybe_draw_sidebar();
    }

    void CursesGui::handle_resize()
    {
        struct winsize w;

        ioctl(0, TIOCGWINSZ, &w);
        COLS  = w.ws_col;
        LINES = w.ws_row;

        this->main << Erase;

        resize_term(LINES, COLS);

        int sidebar_size;

        if (!this->sidebar_visible)
            sidebar_size = 0;
        else
            sidebar_size = 21;

        this->width  = COLS - sidebar_size;
        this->height = LINES - 1;

        this->main.resize(0, 0, this->width, this->height);
        this->sidebar.resize(COLS - sidebar_size, 0, sidebar_size - 1, this->height);
        this->command.resize(0, LINES - 1, COLS, 1);

        this->current_view->center_cursor();

        refresh();

        this->maybe_draw_sidebar();
    }

    void CursesGui::maybe_draw_sidebar()
    {
        if (this->sidebar_visible)
            this->current_view->draw_sidebar(this->squareSource, this->cursors);
    }

    void CursesGui::draw()
    {
        handle_input();
        draw_board();
        this->maybe_draw_sidebar();

        if (!command_mode)
        {
            // readline weirdness
            in.async_read_some(boost::asio::null_buffers(), std::bind(&CursesGui::draw, this));
        }
    }

    void CursesGui::save_png(std::string args)
    {
        if (args.empty())
        {
            command << Erase << "Error: you must enter a filename!" << Refresh;
            return;
        }

        ev_save_image(args + ".png");
        command << Erase << "Board image written to '" << args.c_str() << ".png'." << Refresh;
    }

    void CursesGui::switch_views()
    {
        this->current_view->switched_out_handler();
        if (this->chunk_view_enabled)
        {
            this->current_view = &this->boardview;
        }
        else
        {
            this->current_view = &this->chunkview;
        }

        this->current_view->switched_in_handler();
        this->chunk_view_enabled = !this->chunk_view_enabled;
    }

    void CursesGui::handle_input(int ch)
    {
        switch (ch)
        {
            case 'v':
                this->switch_views();
                break;

            case 'q':
                this->ev_exit();
                break;

            case 'p':
                this->save_png("board");
                break;

            case 't':
                this->sidebar_visible = !this->sidebar_visible;
                handle_resize();
                break;

            case KEY_RESIZE:
                handle_resize();
        }

        this->current_view->handle_input(ch);
    }

    void CursesGui::handle_input()
    {
        int ch;

        while ((ch = getch()) != ERR)
        {
            handle_input(ch);
        }
    }

    void CursesGui::draw_board()
    {
        this->current_view->draw_main(this->squareSource, this->cursors);
    }

    void CursesGui::start()
    {
        this->handle_resize();
        this->current_view->center_cursor(0, 0);
        this->draw_board();
        in.async_read_some(boost::asio::null_buffers(), std::bind(&CursesGui::draw, this));
        io_service.run();
    }
}  // namespace nm::curses
