#ifndef NM_CHUNKSOURCE_HPP
#define NM_CHUNKSOURCE_HPP

#include <unordered_map>
#include "Typedefs.hpp"
#include "Chunk.hpp"

namespace nm
{
	typedef std::unordered_map<Coordinates, Chunk, nm::int_pair_hash<Coordinates>> ChunkList;
	class ChunkSource
	{
		public:
			virtual ~ChunkSource()
			{
			};
			virtual const ChunkList& get_chunks() const = 0;
			virtual std::optional<Chunk* const> get_chunk(const Coordinates& coordinates) = 0;
	};
}

#endif //NM_CHUNKSOURCE_HPP
