#ifndef _NM_GAME_H_
#define _NM_GAME_H_

#include "Board.hpp"

#include <chrono>
#include <iostream>
#include <nm/SquareSource.hpp>
#include <nm/Typedefs.hpp>
#include <nm/Utils.hpp>
#include <optional>
#include <unordered_set>

struct open_results
{
    const nm::Coordinates affected_chunk;
    const bool mine_opened;
};

namespace nm
{
    class Game
    {
      private:
        void initialize();
        std::optional<open_results> open_square(int x, int y);
        void number_square(const Coordinates& c);
        void renumber_chunk(const Coordinates& c, const bool renumber_original);
        void compute_overflagging(const Coordinates& c);
        bool completely_flagged(int x, int y);
        void save_game() const;
        bool save = false;
        std::chrono::time_point<std::chrono::system_clock> last_autosave;

      public:
        Game();
        Game(Board&& board);

        Game(const Game& other);
        Game(Game&& other) noexcept;

        Game& operator=(const Game& other);
        Game& operator=(Game&& other) noexcept;

        ~Game();
        Board board;
        void flag_square_handler(int x, int y);
        void open_square_handler(const int x, const int y, const bool check_flags = true);

        void save_on_destruct()
        {
            save = true;
        };

        std::unordered_set<Coordinates, int_pair_hash<Coordinates>> updated_chunks;
        void maybe_autosave();
        void save_game(std::string save_path) const;
    };
}  // namespace nm

#endif  //_NM_GAME_H_
