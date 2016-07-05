#include "Chunk.hpp"

namespace nm
{
    Square& Chunk::get(Coordinates coordinates) {
        return chunk[coordinates.y()][coordinates.x()];
    }
}	Chunk Chunk::transform_copy(std::function<void(Square&)> functor)
	Chunk Chunk::transform_copy(std::function<void(Square&)> functor)
	{
		Chunk temp = *this;
		std::for_each(temp.chunk.begin(), temp.chunk.end(), [&functor](std::array<Square, NM_CHUNK_SIZE>& row)
		{
			std::for_each(row.begin(), row.end(), functor);
		});
		return temp;
	}
