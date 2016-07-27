#ifndef CURSESSTREAMBUF_HPP
#define CURSESSTREAMBUF_HPP

#include <streambuf>
#include <ncurses.h>
#include <boost/noncopyable.hpp>

namespace nm {
	class CursesStreamBuf : public std::wstreambuf, virtual public boost::noncopyable {
	public:
		explicit CursesStreamBuf(WINDOW *new_win);

	private:
		int_type overflow(int_type ch);
	public:
		WINDOW *win;
	};
}

#endif // CURSESSTREAMBUF_HPP
