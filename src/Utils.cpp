#include <nm/Utils.hpp>

namespace nm
{
	namespace utils
	{
		const Coordinates to_chunk_coordinates(const Coordinates& c)
		{
			int x = ((int)std::floor(c.x() / (double)NM_CHUNK_SIZE));
			int y = ((int)std::floor(c.y() / (double)NM_CHUNK_SIZE));

			return {x, y};
		}

		const Coordinates to_global_coordinates(const Coordinates& local, const Coordinates& chunk)
		{
			return {local.x() + (NM_CHUNK_SIZE * chunk.x()), local.y() + (NM_CHUNK_SIZE * chunk.y())};
		}

		void for_around(int x, int y, const std::function<void (int, int)>& functor)
		{
			static int around_offsets[3] = {-1, 0, 1};

			for (auto&& xoff : around_offsets)
			{
				for (auto&& yoff : around_offsets)
				{
					functor(x + xoff, y + yoff);
				}
			}
		}
	}
}
