#include <nm/SaveLoad.hpp>
#include <nm/Board.hpp>

#include <boost/log/trivial.hpp>
#include <fstream>

using nlohmann::json;

namespace nm
{
	ChunkList Loader::json_to_chunks(const json& chunk_map)
	{
		ChunkList chunkList;

		for (auto &pair : json::iterator_wrapper(chunk_map))
		{
			std::string key = pair.key();
			size_t split_point = key.find(":");

			int x = std::stoi(key.substr(0, split_point));
			int y = std::stoi(key.substr(split_point+1));

			Chunk chunk;
			chunk.deserialize(pair.value().get<std::string>().c_str());
			chunkList[Coordinates(x, y)] = chunk;
		}

		return chunkList;
	}

	boost::optional<Game> Loader::loadGame(const std::string& filename)
	{
		std::ifstream stream;
		stream.open(filename.c_str(), stream.in);

		if (!stream.is_open())
		{
			BOOST_LOG_TRIVIAL(error) << "[Loader] Unable to open savegame, does '" << filename << "' exist?";
			return boost::optional<Game>();
		}

		json saveGame(stream);

		if (saveGame["version"].get<int>() != NM_FORMAT_VERSION)
		{
			BOOST_LOG_TRIVIAL(error) << "[Loader] Save game uses NM_FORMAT_VERSION " << saveGame["version"].get<int>()
			   << ", instead of required " << NM_FORMAT_VERSION;
			return boost::optional<Game>();
		}

		ChunkList chunks = json_to_chunks(saveGame["chunks"]);

		Board board;
		board.chunks = chunks;

		BOOST_LOG_TRIVIAL(info) << "[Loader] Successfully loaded savegame '" << filename << "'!";

		return boost::optional<Game>(Game(board));
	}
}
