#include <nm/ChunkGenerator.hpp>
#include <nm/Square.hpp>
#include <random>
#include <iostream>
#include <cmath>

#include <boost/log/trivial.hpp>

namespace nm
{
	ChunkGenerator::ChunkGenerator(double mean_density, double variation) : mean_density(mean_density), variation(variation)
	{
	};

	Chunk ChunkGenerator::generate() const
	{
		Chunk chunk;

		// rd is used to seed the Mersenne Twister
		std::random_device rd;
		std::mt19937 generator(rd());

		std::uniform_real_distribution<> density_gen(mean_density - variation,
													 mean_density + variation);
		std::uniform_int_distribution<> coord_gen(0, NM_CHUNK_SIZE - 1);

		double mine_density = density_gen(generator);
		int nmines = std::floor(mine_density * (NM_CHUNK_SIZE*NM_CHUNK_SIZE));

		BOOST_LOG_TRIVIAL(info) << "[ckgen] generating chunk with " << nmines << " mines"
				  << " (" << mine_density * 100.0 << "%)";

		while (nmines > 0)
		{
			int rand_x = coord_gen(generator);
			int rand_y = coord_gen(generator);

			Square& current_square = chunk.get({rand_x, rand_y});
			if (!current_square.is_mine)
			{
				current_square.is_mine = 1;
				nmines--;
			}
		}

		return chunk;
	}
}
