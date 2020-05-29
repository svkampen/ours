#include <nm/Window.hpp>

namespace nm
{
    Window::Window(int sx, int sy, int ex, int ey): CursesStream(newwin(ey, ex, sy, sx))
    {
        this->startx = sx;
        this->starty = sy;
        this->cols   = ex;
        this->lines  = ey;
    }

    Window::Window(): Window(0, 0, 0, 0) {};

    Window::Window(WINDOW* const win): CursesStream(win)
    {
        getbegyx(win, this->starty, this->startx);
        getmaxyx(win, this->lines, this->cols);
    }

    void Window::resize(int x, int y, int cols, int lines)
    {
        wresize(this->win, lines, cols);

        this->startx = x;
        this->starty = y;
        this->cols   = cols;
        this->lines  = lines;

        mvwin(this->win, y, x);
    }

    int Window::getx()
    {
        int x, y;
        getyx(this->win, y, x);
        return x;
    }

    int Window::gety()
    {
        int y, x;
        getyx(this->win, y, x);
        return y;
    }

    void Window::remove()
    {
        delwin(this->win);
    }
}  // namespace nm
