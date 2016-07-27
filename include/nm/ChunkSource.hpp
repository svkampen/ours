#ifndef NM_CHUNKSOURCE_HPP
#define NM_CHUNKSOURCE_HPP

#include <unordered_map>
#include "Typedefs.hpp"
#include "Chunk.hpp"

namespace nm
{
	typedef std::unordered_map<Coordinates, Chunk> ChunkList;
	class ChunkSource
	{
		public:
			virtual ~ChunkSource()
			{
			};
			virtual const ChunkList& get_chunks() const = 0;
	};
}

#endif //NM_CHUNKSOURCE_HPP
