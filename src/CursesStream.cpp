#include <nm/CursesStream.hpp>

namespace nm
{
#pragma GCC diagnostic ignored "-Wreorder"
    CursesStream::CursesStream(WINDOW* new_win):
        CursesStreamBuf(new_win),
        std::wios(0),
        std::wostream(static_cast<CursesStreamBuf*>(this)),
        win(new_win)
    {
    }

    CursesStream::~CursesStream()
    {
        sync();
    }

    CursesStream::operator WINDOW*()
    {
        return win;
    }

    AttrSet Bold(A_BOLD), Normal(A_NORMAL);
}  // namespace nm
