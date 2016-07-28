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
			Chunk generate();


			void setMeanDensity(double density)
			{
				this->mean_density = density;
			}

			void setVariation(double variation)
			{
				this->variation = variation;
			}
	};
}

#endif // CHUNKGENERATOR_HPP
