#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "Square.hpp"
#include "Typedefs.hpp"

#define NM_CHUNK_SIZE 32

namespace nm
{
    class Chunk
    {
        private:
            std::array<std::array<Square, NM_CHUNK_SIZE>, NM_CHUNK_SIZE> chunk;
        public:
            Square& get(Coordinates coordinates);
    };
}

#endif // CHUNK_HPP