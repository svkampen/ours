#ifndef _NM_GUI_
#define _NM_GUI_

#include <boost/signals2.hpp>
#include <ours.pb.h>
#include <nm/ChunkSquareSource.hpp>
#include <nm/Game.hpp>
#include <nm/Typedefs.hpp>
#include <optional>
#include <string_view>

namespace nm
{
    class Gui
    {
      protected:
        ChunkSquareSource& squareSource;
        CursorData self_cursor {};
        Gui(ChunkSquareSource& squareSource);

      public:
        SquareEvent ev_square_open;
        SquareEvent ev_square_flag;
        SquareEvent ev_cursor_move;

        boost::signals2::signal<void()> ev_exit;

        virtual void handle_input(int ch) = 0;
        virtual void handle_command_input(std::string_view) {};

        virtual const ChunkSquareSource& get_squaresource();
        virtual const CursorData& get_cursor();

        virtual void display_command(std::string_view cmd);
        virtual void display_command(std::string_view cmd, std::optional<int> endpos);

        virtual void cursor_move_handler(const message::MessageWrapper& mwpr) = 0;
        virtual void player_quit_handler(const message::MessageWrapper& mwpr) = 0;
        virtual void new_player_handler(const nm::message::MessageWrapper& mwpr);
        virtual void new_player_handler(const nm::message::Player& player) = 0;

        virtual void draw_board() = 0;
        virtual void draw()       = 0;
        virtual void handle_resize();

        virtual void start() = 0;
    };
}  // namespace nm

#endif  // _NM_GUI_
