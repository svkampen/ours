#ifndef _NM_CURSESGUI_
#define _NM_CURSESGUI_

#include <ncurses.h>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <nm/Gui.hpp>
#include <nm/Game.hpp>
#include <nm/ChunkSquareSource.hpp>
#include <nm/Flag.hpp>
#include <nm/View.hpp>
#include <nm/BoardView.hpp>
#include <nm/ChunkView.hpp>
#include <nm/Client.hpp>
#include <nm/Window.hpp>
#include <nm/GuileInterpreter.hpp>

#include <netmine.pb.h>
#include <unordered_map>

#include <boost/asio.hpp>

namespace std
{
	template<>
	struct hash<nm::message::CursorMove>
	{
		typedef nm::message::CursorMove argument_type;
		std::size_t operator()(argument_type const& c) const
		{
			std::size_t const h1(std::hash<int32_t>()(c.x()));
			std::size_t const h2(std::hash<int32_t>()(c.y()));
			std::size_t const h3(std::hash<int32_t>()(c.id()));

			return (h1 ^ h2) ^ h3;
		}
	};

	template<>
	struct equal_to<nm::message::CursorMove>
	{
		typedef nm::message::CursorMove T;
		bool operator()(const T& lhs, const T& rhs) const
		{
			return (lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.id() == rhs.id());
		}
	};
}

namespace nm::curses
{
	void init_curses();

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
		friend class GuileInterpreter;

		private:
			nm::Window main, sidebar, command;

            nm::GuileInterpreter interpreter;
			boost::asio::posix::stream_descriptor in;
            boost::asio::io_service& io_service;

			std::unordered_map<int32_t, CursorData> cursors;

			bool chunk_view_enabled = false;
			nm::BoardView boardview;
			nm::ChunkView chunkview;
			nm::View *current_view;

			int width;
			int height;

			bool border_enabled = false;
			bool command_mode = false;

			std::stringstream command_buffer;

			void switch_views();
			void start_command_mode(std::string pre_input = "");

			void draw_open_square(int x, int y, Square& square);
			void draw_closed_square(int x, int y);
			void draw_flag_square(int x, int y, Square& square);
			static void save_png(CursesGui*, std::string);
			void handle_input();

		public:
			SquareEvent ev_square_open;
			SquareEvent ev_square_flag;
			SquareEvent ev_cursor_move;
			boost::signals2::signal<void (std::string)> ev_save_image;
			boost::signals2::signal<void ()> ev_exit;

			CursesGui(boost::asio::io_service& io_service, nm::ChunkSquareSource& squareSource);

            void handle_input(int ch);
            void handle_command_input(std::string_view cmd);

            void display_command(std::string_view cmd);
            void display_command(std::string_view cmd, std::optional<int> endpos);

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
