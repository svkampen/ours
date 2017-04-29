#include <nm/CursesStreamBuf.hpp>

namespace nm {
	CursesStreamBuf::CursesStreamBuf(WINDOW * new_win) : win(new_win)
	{	
    	setp(nullptr, nullptr);
	}

	CursesStreamBuf::int_type CursesStreamBuf::overflow(int_type ch)
	{
		wchar_t wch = static_cast<wchar_t>(ch);
		if (win->_curx == win->_maxx && nowrap)
			return traits_type::not_eof(ch);
		waddnwstr(win, &wch, 1);
	    return traits_type::not_eof(ch);
	}
}
