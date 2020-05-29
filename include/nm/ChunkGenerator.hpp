#ifndef CHUNKGENERATOR_HPP
#define CHUNKGENERATOR_HPP

#include "Chunk.hpp"

namespace nm
{
    class ChunkGenerator
    {
      private:
        double mean_density, variation;

      public:
        ChunkGenerator(double mean_density, double variation);

        Chunk generate() const;

        void setMeanDensity(double density)
        {
            this->mean_density = density;
        }

        void setVariation(double variation)
        {
            this->variation = variation;
        }
    };
}  // namespace nm

#endif  // CHUNKGENERATOR_HPP
