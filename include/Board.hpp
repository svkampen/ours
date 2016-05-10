#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cstdint>
#include <unordered_map>

#include "Typedefs.hpp"
#include "Square.hpp"
#include "Chunk.hpp"

namespace nm
{
    class Board
    {
        private:
            std::unordered_map<Coordinates, Chunk> chunks;

        public:
            Square& get(Coordinates coordinates);
    };
};

#endif // BOARD_HPP
