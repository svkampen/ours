#ifndef NM_CHUNKSQUARESOURCE_HPP
#define NM_CHUNKSQUARESOURCE_HPP

#include "ChunkSource.hpp"
#include "SquareSource.hpp"

namespace nm
{
    class ChunkSquareSource : public ChunkSource, public SquareSource
    {
    };
}  // namespace nm

#endif
