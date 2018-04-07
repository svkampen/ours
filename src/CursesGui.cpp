#include <nm/CursesGui.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Flag.hpp>
#include <nm/Utils.hpp>

#include <sys/ioctl.h>
#include <signal.h>

#include <boost/log/trivial.hpp>

#include <readline/readline.h>
#include <readline/history.h>

extern void hook_redisplay();
extern int hook_input_avail();
extern void rl_callback_handler(char* line);

namespace nm::curses
{
	void init_curses()
	{
		setlocale(LC_ALL, "");
		initscr();
		srand(time(NULL));

		if (has_colors())
		{
			start_color();
			use_default_colors();
			init_pair(1, COLOR_WHITE,	-1);
			init_pair(2, COLOR_BLUE,	-1);
			init_pair(3, COLOR_CYAN,	-1);
			init_pair(4, COLOR_GREEN,	-1);
			init_pair(5, COLOR_MAGENTA, -1);
			init_pair(6, COLOR_RED,		-1);

			// Inverted colors

			init_pair(7, COLOR_BLACK,	COLOR_WHITE);
			init_pair(8, COLOR_BLUE,	COLOR_WHITE);
			init_pair(9, COLOR_CYAN,	COLOR_WHITE);
			init_pair(10,COLOR_GREEN,	COLOR_WHITE);
			init_pair(11,COLOR_MAGENTA,	COLOR_WHITE);
			init_pair(12,COLOR_RED,		COLOR_WHITE);

			// Red backgrounded

			init_pair(13,COLOR_BLACK,	COLOR_RED);
			init_pair(14,COLOR_BLUE,	COLOR_RED);
			init_pair(15,COLOR_CYAN,	COLOR_RED);
			init_pair(16,COLOR_GREEN,	COLOR_RED);
			init_pair(17,COLOR_MAGENTA,	COLOR_RED);
			init_pair(18,COLOR_WHITE,	COLOR_RED);

			init_pair(19, COLOR_BLACK, COLOR_GREEN);
		}

		noecho();
		cbreak();
		keypad(stdscr, TRUE);
		nodelay(stdscr, TRUE);
		curs_set(0);
	}

	CursesGui::CursesGui(boost::asio::io_service& io_service, ChunkSquareSource& squareSource)
		: Gui(squareSource), main(0, 0, COLS - 21, LINES - 1), sidebar(COLS - 20, 0, 20, LINES - 1),
			in(io_service, ::dup(STDIN_FILENO)), io_service(io_service),
            boardview(self_cursor, main, sidebar, ev_square_open, ev_square_flag, ev_cursor_move),
			chunkview(self_cursor, main, sidebar), current_view(&boardview), command(0, LINES - 1, COLS, 1)
	{
		main.nowrap = true;
		width = height = 0;
		handle_resize();

		this->current_view->center_cursor(0, 0);

		this->draw_board();
		in.async_read_some(boost::asio::null_buffers(), boost::bind(&CursesGui::draw, this));
	}

	void CursesGui::player_quit_handler(const message::MessageWrapper& mwpr)
	{
		auto player = mwpr.player();
		cursors.erase(player.id());
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void CursesGui::new_player_handler(const message::Player& player)
	{
		BOOST_LOG_TRIVIAL(info) << "Adding cursor for new player with ID " << player.id();
		cursors[player.id()] = {
			.x =  player.x(),
			.y = player.y(),
			.offset_x = 0,
			.offset_y = 0,
			.color = rand() % 4 + 2
		};
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void CursesGui::cursor_move_handler(const message::MessageWrapper& mwpr)
	{
		auto msg = mwpr.cursormove();
		BOOST_LOG_TRIVIAL(info) << "Moving cursor for " << msg.id();
		auto &&data = cursors[msg.id()];
		data.x = msg.x();
		data.y = msg.y();
		draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void CursesGui::handle_resize()
	{
		struct winsize w;

		ioctl(0, TIOCGWINSZ, &w);
		COLS = w.ws_col;
		LINES = w.ws_row;

        this->main << Erase;

		resize_term(LINES, COLS);

		this->width = COLS - 21;
		this->height = LINES - 1;

		this->main.resize(0, 0, this->width, this->height);
		this->sidebar.resize(COLS - 20, 0, 20, this->height);
		this->command.resize(0, LINES - 1, COLS, 1);

		this->current_view->center_cursor();

		refresh();

		this->current_view->draw_sidebar(this->squareSource, this->cursors);
	}

	void CursesGui::draw()
	{
        handle_input();
        draw_board();
		this->current_view->draw_sidebar(this->squareSource, this->cursors);

		if (!command_mode)
		{
			// readline weirdness
			in.async_read_some(boost::asio::null_buffers(), boost::bind(&CursesGui::draw, this));
		}
	}

	void CursesGui::save_png(CursesGui* gui, std::string args)
	{
		if (args.empty())
		{
			gui->command << Erase << "Error: you must enter a filename!" << Refresh;
			return;
		}

		gui->ev_save_image(args + ".png");
		gui->command << Erase << "Board image written to '" << args.c_str() << ".png'." << Refresh;
	}

	void CursesGui::switch_views()
	{
		this->current_view->switched_out_handler();
		if (this->chunk_view_enabled)
		{
			this->current_view = &this->boardview;
		} else
		{
			this->current_view = &this->chunkview;
		}

		this->current_view->switched_in_handler();
		this->chunk_view_enabled = !this->chunk_view_enabled;
	}

    void CursesGui::handle_input(int ch)
    {
        switch(ch)
        {
            case 'v':
                this->switch_views();
                break;

            case 'q':
                this->ev_exit();
                break;

            case 'p':
                this->start_command_mode("(nm-save-png ");
                break;

            case ':':
                this->start_command_mode();
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

	void CursesGui::start_command_mode(std::string pre_input)
	{
		this->command_mode = true;

		curs_set(1);
		echo();
		rl_callback_handler_install("", rl_callback_handler);
		rl_input_available_hook = hook_input_avail;
		rl_redisplay_function = hook_redisplay;

		rl_replace_line(pre_input.c_str(), 1);
		rl_point = rl_end;

		hook_redisplay();

		while (this->command_mode)
			if (hook_input_avail())
				rl_callback_read_char();

		curs_set(0);
		noecho();
	}

	void CursesGui::handle_command_input(std::string_view cmd)
	{
		this->interpreter.run_command(cmd);
		this->command_mode = false;

		rl_callback_handler_remove();

		redrawwin((WINDOW*)main);
		redrawwin((WINDOW*)sidebar);

		this->draw();
	}

    void CursesGui::display_command(std::string_view cmd)
    {
        display_command(cmd, {});
    }

    void CursesGui::display_command(std::string_view cmd, std::optional<int> endpos)
    {
        command << Erase << ":" << cmd.data() << ClrToBot;

        if (endpos)
        {
            command << Move({*endpos % COLS, 0});
        }

        command << Refresh;
    }

	void CursesGui::draw_board()
	{
		this->current_view->draw_main(this->squareSource, this->cursors);
	}

    void CursesGui::start()
    {
        io_service.run();
    }
}
