#ifndef CURSESSTREAMBUF_HPP
#define CURSESSTREAMBUF_HPP

#include <boost/noncopyable.hpp>
#include <ncurses.h>
#include <streambuf>

namespace nm
{
    class CursesStreamBuf : public std::wstreambuf, virtual public boost::noncopyable
    {
      public:
        explicit CursesStreamBuf(WINDOW* new_win);

      private:
        int_type overflow(int_type ch);

      public:
        WINDOW* win;
        bool nowrap = false;
    };
}  // namespace nm

#endif  // CURSESSTREAMBUF_HPP
