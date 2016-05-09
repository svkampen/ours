#include "Chunk.hpp"

namespace nm
{
    Square& Chunk::get(Coordinates coordinates) {
        return chunk[coordinates.y()][coordinates.x()];
    }
}