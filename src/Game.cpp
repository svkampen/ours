#include <boost/log/trivial.hpp>
#include <nm/Config.hpp>
#include <nm/Game.hpp>
#include <nm/SaveLoad.hpp>
#include <nm/Utils.hpp>

using namespace std::literals;

namespace nm
{
    Game::Game(): board()
    {
    }

    Game::~Game()
    {
        this->save_game();
    }

    void Game::initialize()
    {
        this->last_autosave = std::chrono::system_clock::now();
    }

    Game::Game(Board&& board): board(std::move(board))
    {
        initialize();
    };

    Game::Game(const Game& other): board(other.board)
    {
        initialize();
    };

    Game::Game(Game&& other) noexcept: board(std::move(other.board))
    {
        initialize();
    };

    Game& Game::operator=(const Game& other)
    {
        board = other.board;
        initialize();
        return *this;
    }

    Game& Game::operator=(Game&& other) noexcept
    {
        board = std::move(other.board);
        initialize();
        return *this;
    }

    void Game::number_square(const Coordinates& c)
    {
        Square& square = board.get(c);

        if (square.state != SquareState::OPENED)
            return;

        square.number = 0;

        utils::for_around(c.x(), c.y(), [this, &square](int x, int y) {
            if (board.get(x, y).is_mine)
                square.number++;
        });
    }

    std::optional<open_results> Game::open_square(int x, int y)
    {
        Square& square = board.get({x, y});
        if (square.state == SquareState::OPENED)
            return std::optional<open_results>();
        square.state                  = SquareState::OPENED;
        Coordinates chunk_coordinates = nm::utils::to_chunk_coordinates({x, y});

        this->number_square({x, y});

        return open_results {chunk_coordinates, square.is_mine};
    }

    void Game::renumber_chunk(const Coordinates& c, const bool renumber_original = true)
    {
        utils::for_around(c.x(), c.y(), [this, c, renumber_original](int x, int y) {
            if (x == c.x() && y == c.y() && !renumber_original)
                return;

            const Coordinates chunk_current = {x, y};

            for (int local_x = 0; local_x < NM_CHUNK_SIZE; local_x++)
            {
                for (int local_y = 0; local_y < NM_CHUNK_SIZE; local_y++)
                {
                    auto global_coords =
                        nm::utils::to_global_coordinates({local_x, local_y}, chunk_current);
                    this->number_square(global_coords);
                    this->compute_overflagging(global_coords);
                }
            }

            this->updated_chunks.insert(chunk_current);
        });
    }

    void Game::compute_overflagging(const Coordinates& c)
    {
        /* Update overflagging stuff. */
        auto is_overflagged = [this](int x, int y) {
            int number       = 0;
            const Square& sq = board.get(x, y);
            nm::utils::for_around(x, y, [&number, this](int x, int y) {
                if (board.get(x, y).state == SquareState::FLAGGED)
                    number++;
            });
            if (number > sq.number && sq.state == SquareState::OPENED)
                return true;
            return false;
        };

        nm::utils::for_around(c.x(), c.y(), [&is_overflagged, this](int x, int y) {
            board.get(x, y).overflag = is_overflagged(x, y);
        });
    }

    void Game::flag_square_handler(int x, int y)
    {
        Square& square = board.get(x, y);

        if (square.state == SquareState::OPENED)
        {
            // Can't flag an opened square
            return;
        }

        if (square.state == SquareState::FLAGGED)
        {
            square.state = SquareState::CLOSED;
        }
        else
        {
            square.state = SquareState::FLAGGED;
        }

        if (nm::config["show_overflagged"])
        {
            this->compute_overflagging({x, y});
            if (nm::utils::on_chunk_boundary(nm::utils::to_local_coordinates({x, y})))
            {
                auto chunk_coordinates = nm::utils::to_chunk_coordinates({x, y});
                nm::utils::for_around(chunk_coordinates.x(), chunk_coordinates.y(), [this](int cx, int cy) {
                        updated_chunks.insert({cx, cy});
                });
            }
        }

        updated_chunks.insert(nm::utils::to_chunk_coordinates({x, y}));

        maybe_autosave();
    }

    bool Game::completely_flagged(int x, int y)
    {
        int nflags     = 0;
        Square& square = board.get(x, y);
        utils::for_around(x, y, [this, &nflags](int x, int y) {
            Square& s = board.get(x, y);
            if (s.state == SquareState::FLAGGED)
                nflags++;
        });

        // Necessary because BOOST_LOG_TRIVIAL takes references, and
        // square.number is a bitfield (you can only take const references)
        BOOST_LOG_TRIVIAL(info) << "Number of flags: " << nflags
                                << ", square number: " << (const int) square.number;
        return (nflags == square.number);
    }

    void Game::open_square_handler(const int x, const int y, const bool check_flags)
    {
        std::unordered_set<Coordinates, int_pair_hash<Coordinates>> affected_chunks;

        Square& square         = board.get(x, y);
        SquareState orig_state = square.state;

        if (square.state == SquareState::OPENED)
        {
            if (check_flags && completely_flagged(x, y))
            {
                utils::for_around(x, y, [this, &affected_chunks](int x, int y) {
                    open_square_handler(x, y, false);
                });
            }
        }

        if (square.state == SquareState::FLAGGED)
        {
            // Do nothing, as flagged squares cannot be opened
            return;
        }

        auto result = open_square(x, y);
        if (!result)
            return;

        updated_chunks.insert(result->affected_chunk);

        if (result->mine_opened)
        {
            // If a mine has been opened, regenerate the chunk.
            board.regenerate_chunk(result->affected_chunk);

            // Don't renumber the original chunk, since it is completely closed
            // off anyway.
            this->renumber_chunk(result->affected_chunk, false);
            return;
        }

        // If the square has no mines around it and it was not originally open,
        // open the squares around it.
        if (square.number == 0 && orig_state != SquareState::OPENED)
        {
            utils::for_around(x, y, [this](int x, int y) { open_square_handler(x, y, false); });
        }

        maybe_autosave();
    }

    void Game::save_game() const
    {
        this->save_game(nm::config["save_path"].get<std::string>());
    }

    void Game::save_game(std::string save_path) const
    {
        if (!save)
            return;

        try
        {
            nm::Saver::saveGame(this->board, save_path);
        }
        catch (std::exception& e)
        {
            std::cerr << "Received exception while saving game: " << e.what() << std::endl;
        }
    }

    void Game::maybe_autosave()
    {
        using namespace std::chrono;
        if (duration_cast<seconds>(system_clock::now() - this->last_autosave).count() > 300)
        {
            static const std::string save_path =
                nm::config["save_path"].get<std::string>() + ".autosave"s;
            this->save_game(save_path);
            this->last_autosave = system_clock::now();
        }
    }
}  // namespace nm
