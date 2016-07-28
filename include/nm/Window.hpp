#ifndef _IRC_WINDOW_HPP_
#define _IRC_WINDOW_HPP_

#ifndef NCURSES_NOMACROS
#define NCURSES_NOMACROS
#endif // NCURSES_NOMACROS

#include <ncurses.h>
#include <string>
#include <functional>
#include <boost/noncopyable.hpp>
#include "CursesStream.hpp"

namespace nm {

	class Window : public CursesStream, virtual public boost::noncopyable {
		public:
			int startx, starty;
			int lines, cols;

			std::function<void (nm::Window& handle)> refresh_fn;

			Window(int sx, int sy, int ex, int ey);
			Window(WINDOW *win);
			Window();

			void setscroll(bool b) { scrollok((WINDOW*)this, b); idlok((WINDOW*)this, b); };
			void resize(int x, int y, int cols, int lines);
			void remove();

			int getx();
			int gety();

	};
}

#endif
