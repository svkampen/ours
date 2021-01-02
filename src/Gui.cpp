#include <nm/Gui.hpp>

namespace nm
{
    Gui::Gui(ChunkSquareSource& squareSource): squareSource(squareSource) {};

    const ChunkSquareSource& Gui::get_squaresource()
    {
        return squareSource;
    }

    const CursorData& Gui::get_cursor()
    {
        return self_cursor;
    }

    void Gui::display_command(std::string_view) {};
    void Gui::display_command(std::string_view, std::optional<int>) {};

    void Gui::new_player_handler(const nm::message::MessageWrapper& mwpr)
    {
        new_player_handler(mwpr.player());
    };

    void Gui::handle_resize() {};
}  // namespace nm