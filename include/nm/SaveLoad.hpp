#ifndef NM_SAVELOAD_HPP
#define NM_SAVELOAD_HPP

#include "Game.hpp"

#include <boost/optional.hpp>
#include <json.hpp>

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
			static boost::optional<Game> loadGame(const std::string& filename);
	};
}

#endif // NM_SAVELOAD_HPP
