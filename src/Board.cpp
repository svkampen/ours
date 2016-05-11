#include "Board.hpp"
#include "ChunkGenerator.hpp"

#include <iostream>

namespace nm {

    int32_t modulo(int32_t n, int32_t mod) {
        // Since C++'s modulo behavior with negative numbers is dependent on the
        // implementation, this function makes sure modulo behavior is consistent
        // across implementations.
        int32_t temp = n % mod;
        return (temp < 0) ? (mod - temp) : temp;
    }

    Square& Board::get(Coordinates coordinates)
    {
        simmo::vector<int32_t, 2> local_coords = {modulo(coordinates.x(), NM_CHUNK_SIZE),
                                                  modulo(coordinates.y(), NM_CHUNK_SIZE)};

        Coordinates chunk_coordinates = coordinates - local_coords;

        auto chunk_iterator = chunks.find(chunk_coordinates);
        if (chunk_iterator == chunks.end()) {
            // The chunk does not exist, so we generate it.
            std::cerr << "[board] generating chunk at (" << chunk_coordinates.x()
                      << ", " << chunk_coordinates.y() << ")" << std::endl;
            chunks[chunk_coordinates] = ChunkGenerator::generate(0.15, 0.03);
        }

        return chunks[chunk_coordinates].get(local_coords);
    }
}
