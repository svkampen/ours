#ifndef _NM_CURSES_SETUP_TEARDOWN_
#define _NM_CURSES_SETUP_TEARDOWN_

#include <clocale>
#include <cstdlib>
#include <ctime>
#include <ncurses.h>

namespace nm
{
    struct CursesSetupTeardown
    {
        CursesSetupTeardown()
        {
            std::setlocale(LC_ALL, "");
            initscr();
            std::srand(std::time(NULL));

            if (has_colors())
            {
                start_color();
                use_default_colors();
                init_pair(1, COLOR_WHITE, -1);
                init_pair(2, COLOR_BLUE, -1);
                init_pair(3, COLOR_CYAN, -1);
                init_pair(4, COLOR_GREEN, -1);
                init_pair(5, COLOR_MAGENTA, -1);
                init_pair(6, COLOR_RED, -1);

                // Inverted colors

                init_pair(7, COLOR_BLACK, COLOR_WHITE);
                init_pair(8, COLOR_BLUE, COLOR_WHITE);
                init_pair(9, COLOR_CYAN, COLOR_WHITE);
                init_pair(10, COLOR_GREEN, COLOR_WHITE);
                init_pair(11, COLOR_MAGENTA, COLOR_WHITE);
                init_pair(12, COLOR_RED, COLOR_WHITE);

                // Red backgrounded

                init_pair(13, COLOR_BLACK, COLOR_RED);
                init_pair(14, COLOR_BLUE, COLOR_RED);
                init_pair(15, COLOR_CYAN, COLOR_RED);
                init_pair(16, COLOR_GREEN, COLOR_RED);
                init_pair(17, COLOR_MAGENTA, COLOR_RED);
                init_pair(18, COLOR_WHITE, COLOR_RED);

                init_pair(19, COLOR_BLACK, COLOR_GREEN);
            }

            noecho();
            cbreak();
            keypad(stdscr, TRUE);
            nodelay(stdscr, TRUE);
            curs_set(0);
        }

        ~CursesSetupTeardown()
        {
            endwin();
        }
    };
}  // namespace nm

#endif
