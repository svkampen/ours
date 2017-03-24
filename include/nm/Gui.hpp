#ifndef _NM_GUI_
#define _NM_GUI_

#include <ncurses.h>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <nm/Game.hpp>
#include <nm/ChunkSquareSource.hpp>
#include <nm/Flag.hpp>
#include <nm/View.hpp>
#include <nm/BoardView.hpp>
#include <nm/ChunkView.hpp>
#include <nm/Client.hpp>
#include <nm/Window.hpp>
#include <nm/CursorData.hpp>

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

namespace nm
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
	class Gui : public boost::enable_shared_from_this<Gui>
	{
		private:
			nm::Window main, sidebar;

			ChunkSquareSource &squareSource;
			boost::asio::posix::stream_descriptor in;

			std::unordered_map<int32_t, CursorData> cursors;

			bool chunk_view_enabled = false;
			nm::BoardView boardview;
			nm::ChunkView chunkview;
			nm::View *current_view;

			int width;
			int height;

			CursorData self_cursor {};
			bool border_enabled = false;

			void center_cursor();
			void center_cursor(int global_x, int global_y);

			void switch_views();

			void draw_open_square(int x, int y, Square& square);
			void draw_closed_square(int x, int y);
			void draw_flag_square(int x, int y, Square& square);
			void save_png();
		public:
			SquareEvent ev_square_open;
			SquareEvent ev_square_flag;
			SquareEvent ev_cursor_move;
			boost::signals2::signal<void ()> ev_save_image;
			boost::signals2::signal<void ()> ev_exit;

			Gui(boost::asio::io_service& io_service, ChunkSquareSource& squareSource);

			bool handle_input();
			void cursor_move_handler(const message::CursorMove& msg);
			void player_quit_handler(const message::Player& player);
			void new_player_handler(const message::Player& player);
			void draw_board();
			void handle_resize();
			void draw();
	};
}

#endif //_NM_GUI_
