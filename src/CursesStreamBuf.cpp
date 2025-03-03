#include <nm/CursesStreamBuf.hpp>

namespace nm
{
    CursesStreamBuf::CursesStreamBuf(WINDOW* new_win): win(new_win)
    {
        setp(nullptr, nullptr);
    }

    CursesStreamBuf::int_type CursesStreamBuf::overflow(int_type ch)
    {
        int maxx = getmaxx(win);
        int x = getcurx(win);
        wchar_t wch = static_cast<wchar_t>(ch);
        if (x == (maxx - 1) && nowrap)
            return traits_type::not_eof(ch);
        waddnwstr(win, &wch, 1);
        return traits_type::not_eof(ch);
    }
}  // namespace nm
