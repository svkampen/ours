#include <boost/log/trivial.hpp>
#include <ncurses.h>
#include <nm/BoardView.hpp>
#include <nm/Config.hpp>
#include <nm/Typedefs.hpp>
#include <nm/Utils.hpp>

namespace nm
{
    BoardView::BoardView(CursorData& cursorData, Window& main, Window& sidebar,
                         const SquareEvent& open, const SquareEvent& flag, const SquareEvent& move):
        ev_square_open(open),
        ev_square_flag(flag),
        ev_cursor_move(move),
        View(cursorData, main, sidebar) {};

    void BoardView::draw_main(ChunkSquareSource& squareSource,
                              std::unordered_map<int32_t, CursorData>& others)
    {
        int XViewable = main.cols / 3;
        int YViewable = main.lines;

        main << nm::Erase;

        for (int y = 0; y < YViewable; y++)
        {
            for (int x = 0; x < XViewable; x++)
            {
                main << Move({3 * x + 1, y});
                auto& square = squareSource.get({x + cursor.offset_x, y + cursor.offset_y});
                if (square.is_mine && square.state == SquareState::OPENED)
                {
                    main << L"*";
                }
                else if (square.state == SquareState::OPENED)
                {
                    this->draw_open_square(x, y, square, squareSource);
                }
                else if (square.state == SquareState::FLAGGED)
                {
                    this->draw_flag_square(x, y, square);
                }
                else
                {
                    this->draw_closed_square(x, y, square);
                }
            }
        }

        this->draw_cursors(others);
        this->draw_cursor(squareSource, others);

        main << Refresh;
    }

    void BoardView::draw_cursor(ChunkSquareSource&, CursorMap&)
    {
        main << Move({3 * cursor.x, cursor.y}) << "[" << Move({3 * cursor.x + 2, cursor.y}) << "]";

        main << Refresh;
    }

    void BoardView::center_cursor()
    {
        center_cursor(cursor.x + cursor.offset_x, cursor.y + cursor.offset_y);
    }

    void BoardView::center_cursor(int global_x, int global_y)
    {
        cursor.offset_x = global_x - main.cols / 6;
        cursor.x        = main.cols / 6;

        cursor.offset_y = global_y - main.lines / 2;
        cursor.y        = main.lines / 2;

        this->ev_cursor_move(global_x, global_y);
    }

    HandlerResult BoardView::handle_input(int input_character)
    {
        bool full_redraw = false;
        int control = 0;
        std::string key = keyname(input_character);
        if (key == "kRIT5")
            control = 1, input_character = KEY_RIGHT;
        else if (key == "kLFT5")
            control = 1, input_character = KEY_LEFT;
        else if (key == "kUP5")
            control = 1, input_character = KEY_UP;
        else if (key == "kDN5")
            control = 1, input_character = KEY_DOWN;

        switch (input_character)
        {
            case ' ':
                this->ev_square_open(cursor.offset_x + cursor.x, cursor.offset_y + cursor.y);
                full_redraw = true;
                break;
            case 'c':
                this->center_cursor();
                full_redraw = true;
                break;
            case '0':
                this->center_cursor(0, 0);
                full_redraw = true;
                break;
            case 'b':
                this->border_enabled = !this->border_enabled;
                full_redraw          = true;
                break;
            case 's':
                sticky_flags = !sticky_flags;
                break;
            case 'f':
                if (sticky_flags)
                {
                    handle_sticky_flag();
                }
                else
                {
                    this->ev_square_flag(cursor.offset_x + cursor.x, cursor.offset_y + cursor.y);
                }
                full_redraw = true;
                break;

            /* ARROW KEYS */
            case KEY_LEFT:
                if (cursor.x > (0 + 4 * control))
                {
                    cursor.x -= 1 + 4 * control;
                }
                else
                {
                    cursor.offset_x -= 1 + 4 * control;
                    full_redraw = true;
                }
                break;
            case KEY_RIGHT:
                if (cursor.x < (main.cols / 3) - (1 + 4 * control))
                {
                    cursor.x += 1 + 4 * control;
                }
                else
                {
                    cursor.offset_x += 1 + 4 * control;
                    full_redraw = true;
                }
                break;
            case KEY_UP:
                if (cursor.y > (0 + 4 * control))
                {
                    cursor.y -= 1 + 4 * control;
                }
                else
                {
                    cursor.offset_y -= 1 + 4 * control;
                    full_redraw = true;
                }
                break;
            case KEY_DOWN:
                if (cursor.y < main.lines - (1 + 4 * control))
                {
                    cursor.y += 1 + 4 * control;
                }
                else
                {
                    cursor.offset_y += 1 + 4 * control;
                    full_redraw = true;
                }
                break;
            default:
                break;
        }

        switch (input_character)
        {
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_UP:
            case KEY_DOWN:
                this->ev_cursor_move(cursor.x + cursor.offset_x, cursor.y + cursor.offset_y);
                break;
            default:
                break;
        }

        if (full_redraw)
            return HandlerResult::DRAW_ALL;

        return HandlerResult::DRAW_CURSORS;
    }

    void BoardView::draw_sidebar(ChunkSquareSource&,
                                 std::unordered_map<int32_t, CursorData>& cursors)
    {
        // clang-format off
        sidebar << nm::Erase << L"        Ours        "
                             << L"        Help        "
                             << L"┌──────────────────┐"
                             << L"│ Open       Space │"
                             << L"│ Flag           F │"
                             << L"│ Center         C │"
                             << L"│ Chunk view     V │"
                             << L"│ Center         C │"
                             << L"│ Goto origin    0 │"
                             << L"│ Show chunks    B │"
                             << L"│ Save image     P │"
                             << L"│ Toggle sidebar T │"
                             << L"│ Quit           Q │"
                             << L"└──────────────────┘\n";

        sidebar << L"       Clients      "
                << L"┌──────────────────┐";

        for (auto&& pair : cursors)
        {
            sidebar << L"│ " << nm::AttrOn(COLOR_PAIR(pair.second.color)) << L"o "
                    << nm::AttrOff(COLOR_PAIR(pair.second.color)) << utils::int_to_hex(pair.first)
                    << L"       │";

            sidebar << L"│   (" << pair.second.x << L", " << pair.second.y << ")";

            int line = sidebar.gety();

            sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});
        }

        sidebar << L"└──────────────────┘\n";

        sidebar << L"     Information    "
                << L"┌──────────────────┐"
                << L"│ Chunk " << this->chunk_x() << ", " << this->chunk_y();

        int line = sidebar.gety();
        sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});

        sidebar << L"│ Pos " << this->global_x() << ", " << this->global_y();

        line = sidebar.gety();
        sidebar << nm::Move({19, line}) << L"│" << nm::Move({0, line + 1});

        sidebar << L"└──────────────────┘\n";

        sidebar << nm::Refresh;
        // clang-format on
    }

    void BoardView::handle_sticky_flag()
    {
        if (!have_previous_flag)
        {
            have_previous_flag = true;
            previous_flag      = {cursor.offset_x + cursor.x, cursor.offset_y + cursor.y};
        }
        else
        {
            have_previous_flag = false;
            sticky_flags       = false;
        }
        this->ev_square_flag(cursor.offset_x + cursor.x, cursor.offset_y + cursor.y);
    }

    void BoardView::draw_border(Square& sq, int x, int y)
    {
        if (!border_enabled)
            return;

        int global_x = x + cursor.offset_x;
        int global_y = y + cursor.offset_y;

        bool needs_l_border = global_x % NM_CHUNK_SIZE == 0;
        bool needs_r_border = (global_x + 1) % NM_CHUNK_SIZE == 0;
        bool needs_t_border = false;  // global_y % NM_CHUNK_SIZE == 0;
        bool needs_b_border =
            (global_y + 1) % NM_CHUNK_SIZE == 0 && !needs_r_border && !needs_l_border;

        if (needs_l_border)
        {
            main << Move({3 * x, y}) << AttrOn(COLOR_PAIR(BORDER_COLOR)) << " "
                 << AttrOff(COLOR_PAIR(BORDER_COLOR));
        }

        if (needs_r_border)
        {
            main << Move({3 * x + 2, y}) << AttrOn(COLOR_PAIR(BORDER_COLOR)) << " "
                 << AttrOff(COLOR_PAIR(BORDER_COLOR));
        }

        if (needs_t_border)
        {
            main << Move({3 * x + 1, y});
            char data = winch(main) & A_CHARTEXT;
            data      = isdigit(data) ? data : ' ';
            main << data << "\u035e";
        }

        if (needs_b_border)
        {
            main << Move({3 * x + 1, y});
            char data = winch(main) & A_CHARTEXT;
            main << Move({3 * x, y});
            int color = (sq.state == SquareState::CLOSED) ? 12 : 6;
            main << AttrOn(A_UNDERLINE) << AttrOn(COLOR_PAIR(color)) << " " << data << " "
                 << AttrOff(COLOR_PAIR(color)) << AttrOff(A_UNDERLINE);
        }
    }

    void BoardView::draw_flag_square(int x, int y, Square& square)
    {
        main << Move({3 * x, y}) << " # ";
        this->draw_border(square, x, y);
    }

    void BoardView::draw_closed_square(int x, int y, Square& square)
    {
        main << Move({3 * x, y}) << AttrOn(COLOR_PAIR(7)) << "   " << AttrOff(COLOR_PAIR(7));
        this->draw_border(square, x, y);
    }

    void BoardView::draw_cursors(std::unordered_map<int32_t, CursorData>& cursors)
    {
        int width, height;
        getyx((WINDOW*) main, height, width);

        for (auto iterator = cursors.cbegin(); iterator != cursors.cend(); iterator++)
        {
            int x = iterator->second.x - cursor.offset_x;
            int y = iterator->second.y - cursor.offset_y;

            if (x < 0 || y < 0)
                continue;

            if (x >= width || y >= height)
                continue;

            wattron(main, COLOR_PAIR(iterator->second.color));

            if (wmove(main, y, 3 * x) != ERR)
                waddwstr(main, L"[");
            if (wmove(main, y, 3 * x + 2) != ERR)
                waddwstr(main, L"]");

            wattroff(main, COLOR_PAIR(iterator->second.color));
        }
    }

    void BoardView::draw_open_square(int x, int y, Square& square, ChunkSquareSource& squareSource)
    {
        int color = 0;
        switch (square.number)
        {
            case 0:
                color = -1;
                break;
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                color = square.number + 1;
                break;
            case 6:
            case 7:
                color = 6;
                break;
            case 8:
                color = 5;
                break;
        }

        int global_x = x + cursor.offset_x;
        int global_y = y + cursor.offset_y;

        bool chunk_border = (global_x % NM_CHUNK_SIZE == 0 || global_y % NM_CHUNK_SIZE == 0) &&
                            this->border_enabled;

        chunk_border = chunk_border || square.overflag;
        if (square.overflag)
            color = BORDER_COLOR;

        if (color == -1)
        {
            main << Move({3 * x, y}) << L"   ";
            this->draw_border(square, x, y);

            return;
        }

        main << AttrOn(COLOR_PAIR(color)) << nm::Move({3 * x, y}) << ' '
             << static_cast<char>(square.number + 48) << ' ' << AttrOff(COLOR_PAIR(color));

        this->draw_border(square, x, y);
    }
}  // namespace nm
