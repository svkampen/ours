#ifndef _NM_GUI_
#define _NM_GUI_

#include <ncurses.h>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <nm/Game.hpp>
#include <nm/SquareSource.hpp>
#include <nm/Flag.hpp>
#include <nm/View.hpp>
#include <nm/BoardView.hpp>
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

	class Gui : public boost::enable_shared_from_this<Gui>
	{
		private:
			nm::Window main, sidebar;

			SquareSource &squareSource;
			boost::asio::posix::stream_descriptor in;

			std::unordered_map<int32_t, CursorData> cursors;

			nm::View& current_view;

			nm::BoardView boardview;

			int width;
			int height;

			CursorData self_cursor {};
			bool border_enabled = false;

			void center_cursor();
			void center_cursor(int global_x, int global_y);

			inline int global_x();
			inline int global_y();
			inline int chunk_x();
			inline int chunk_y();
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

			Gui(boost::asio::io_service& io_service, SquareSource& squareSource);

			bool handle_input();
			void cursor_move_handler(const message::CursorMove& msg);
			void player_quit_handler(const message::Player& player);
			void new_player_handler(const message::Player& player);
			void draw_cursors();
			void draw_sidebar();
			void draw_board();
			void handle_resize();
			void draw();
			Square& get(int x, int y) const;
	};
}

#endif //_NM_GUI_
