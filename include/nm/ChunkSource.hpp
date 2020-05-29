#ifndef NM_CHUNKSOURCE_HPP
#define NM_CHUNKSOURCE_HPP

#include "Chunk.hpp"
#include "Typedefs.hpp"

#include <unordered_map>

namespace nm
{
    typedef std::unordered_map<Coordinates, Chunk, nm::int_pair_hash<Coordinates>> ChunkList;
    class ChunkSource
    {
      public:
        virtual const ChunkList& get_chunks() const                                   = 0;
        virtual std::optional<Chunk* const> get_chunk(const Coordinates& coordinates) = 0;
    };
}  // namespace nm

#endif  // NM_CHUNKSOURCE_HPP
