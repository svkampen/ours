#ifndef _IRC_WINDOW_HPP_
#define _IRC_WINDOW_HPP_

#ifndef NCURSES_NOMACROS
#    define NCURSES_NOMACROS
#endif  // NCURSES_NOMACROS

#include "CursesStream.hpp"

#include <boost/noncopyable.hpp>
#include <functional>
#include <ncurses.h>
#include <string>

namespace nm
{
    class Window : public CursesStream, virtual public boost::noncopyable
    {
      public:
        int startx, starty;
        int lines, cols;

        std::function<void(nm::Window& handle)> refresh_fn;

        Window(int sx, int sy, int ex, int ey);
        Window(WINDOW* win);
        Window();

        void setscroll(bool b)
        {
            scrollok((WINDOW*) this, b);
            idlok((WINDOW*) this, b);
        };
        void resize(int x, int y, int cols, int lines);
        void remove();

        int getx();
        int gety();
    };
}  // namespace nm

#endif
