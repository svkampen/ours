#include <nm/SaveLoad.hpp>

#include <boost/log/trivial.hpp>
#include <json.hpp>
#include <fstream>

using nlohmann::json;

namespace nm
{
	bool Saver::saveGame(const Game& game, const std::string& filename)
	{
		json output;
		std::ofstream stream;

		stream.open(filename.c_str(), stream.out);

		if (!stream.is_open())
		{
			BOOST_LOG_TRIVIAL(error) << "[Saver] Unable to open game for saving, do you have permissions to open '" << filename << "'?";
			return false;
		}

		output["version"] = NM_FORMAT_VERSION;

		for (const auto& chunkpair : game.board.chunks)
		{
			std::stringstream stream;
			stream << chunkpair.first.x() << ":" << chunkpair.first.y();

			BOOST_LOG_TRIVIAL(info) << "[Saver] Writing chunk " << stream.str() << " to save file.";
			char *data = chunkpair.second.serialize();
			output["chunks"][stream.str()] = std::string(data, NM_CHUNK_SIZE*NM_CHUNK_SIZE);
		}

		stream << output;

		BOOST_LOG_TRIVIAL(info) << "[Saver] Game saved.";

		return true;
	}
}
