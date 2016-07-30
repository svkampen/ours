#ifndef NM_IMAGESAVER_HPP
#define NM_IMAGESAVER_HPP

#include "ChunkSource.hpp"
#include <png++/png.hpp>

namespace nm
{
	class ImageSaver
	{
		ChunkSource& chunkSource;
		png::image<png::rgb_pixel> image;

		void write_chunks(Coordinates begin, Coordinates end, const ChunkList& chunks);
		void write_chunk(int x, int y, const Chunk& chunk);

		public:
			ImageSaver(ChunkSource& chunkSource);
			void save(const std::string& filename);
	};
}

#endif //NM_IMAGESAVER_HPP
