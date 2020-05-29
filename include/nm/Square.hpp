#ifndef SQUARE_HPP
#define SQUARE_HPP

#include <cstdint>

namespace nm
{
    enum class SquareState : uint8_t
    {
        CLOSED = 0,
        OPENED,
        FLAGGED
    };

    struct Square
    {
        SquareState state : 2;
        bool overflag : 1;
        bool is_mine : 1;
        uint8_t number : 4;
    } __attribute__((packed));
}  // namespace nm

#endif  // SQUARE_HPP
