#ifndef _NM_SQUARESOURCE_H_
#define _NM_SQUARESOURCE_H_

#include "Square.hpp"
#include "Typedefs.hpp"

namespace nm
{
    class SquareSource
    {
      public:
        virtual Square& get(const Coordinates& coordinates) = 0;
        virtual Square& get(int x, int y)                   = 0;

        virtual const Square& get(int x, int y) const                   = 0;
        virtual const Square& get(const Coordinates& coordinates) const = 0;
    };
}  // namespace nm

#endif  //_NM_SQUARESOURCE_H_
