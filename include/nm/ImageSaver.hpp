#ifndef NM_IMAGESAVER_HPP
#define NM_IMAGESAVER_HPP

#include "ChunkSource.hpp"

namespace nm
{
	class ImageSaver
	{
		const ChunkSource& chunkSource;
		public:
			ImageSaver(const ChunkSource& chunkSource);
			void save(const std::string& filename);
	};
}

#endif //NM_IMAGESAVER_HPP
