#ifndef _NM_CURSESGUI_
#define _NM_CURSESGUI_

#include <ncurses.h>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <nm/Gui.hpp>
#include <nm/Game.hpp>
#include <nm/ChunkSquareSource.hpp>
#include <nm/View.hpp>
#include <nm/BoardView.hpp>
#include <nm/ChunkView.hpp>
#include <nm/Window.hpp>

#include <netmine.pb.h>
#include <unordered_map>

#include <boost/asio.hpp>

namespace nm::curses
{
	/**
	 * The class that manages the terminal screen.
	 *
	 * This class sets up the terminal screen, manages data such as the cursors that need to be drawn
	 * and manages its own cursor. Drawing the board/sidebar/etc is delegated to view classes, which
	 * can be swapped out.
	 *
	 * If you're looking for a specific view, see the View virtual class.
	 */
	class CursesGui : public Gui
	{
		private:
			nm::Window main, sidebar, command;

			boost::asio::posix::stream_descriptor in;
            boost::asio::io_service& io_service;

			std::unordered_map<int32_t, CursorData> cursors;

			bool chunk_view_enabled = false;
			bool sidebar_visible = true;
			nm::BoardView boardview;
			nm::ChunkView chunkview;
			nm::View *current_view;

			int width;
			int height;

			bool border_enabled = false;
			bool command_mode = false;

			std::stringstream command_buffer;

			void switch_views();

			void maybe_draw_sidebar();
			void draw_open_square(int x, int y, Square& square);
			void draw_closed_square(int x, int y);
			void draw_flag_square(int x, int y, Square& square);
			void save_png(std::string);
			void handle_input();

		public:
			SquareEvent ev_square_open;
			SquareEvent ev_square_flag;
			SquareEvent ev_cursor_move;
			boost::signals2::signal<void (std::string)> ev_save_image;
			boost::signals2::signal<void ()> ev_exit;

			CursesGui(boost::asio::io_service& io_service, nm::ChunkSquareSource& squareSource);

            void handle_input(int ch);

			void cursor_move_handler(const nm::message::MessageWrapper& mwpr);
			void player_quit_handler(const nm::message::MessageWrapper& mwpr);
            void new_player_handler(const nm::message::Player& player);
			void draw_board();
			void handle_resize();
			void draw();

            void start();
	};
}

#endif //_NM_GUI_
