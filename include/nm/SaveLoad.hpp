#ifndef NM_SAVELOAD_HPP
#define NM_SAVELOAD_HPP

#include "Game.hpp"

#include <json.hpp>
#include <optional>

#define NM_FORMAT_VERSION 1

namespace nm
{
    class Saver
    {
      public:
        static bool saveGame(const Game& game, const std::string& filename);
    };

    class Loader
    {
      private:
        static ChunkList json_to_chunks(const nlohmann::json& chunk_map);

      public:
        static std::optional<Game> loadGame(const std::string& filename);
    };
}  // namespace nm

#endif  // NM_SAVELOAD_HPP
