#ifndef CHUNKGENERATOR_HPP
#define CHUNKGENERATOR_HPP

#include "Chunk.hpp"

namespace nm
{
    class ChunkGenerator
    {
        public:
            static Chunk generate(double mean_density, double variation);
    };
}

#endif // CHUNKGENERATOR_HPP