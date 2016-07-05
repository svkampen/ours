#ifndef _NM_SQUARESOURCE_H_
#define _NM_SQUARESOURCE_H_

#include "Square.hpp"
#include "Typedefs.hpp"

namespace nm {
	class SquareSource
	{
		public:
			virtual ~SquareSource()
			{
			};

			virtual Square& get(Coordinates coordinates) = 0;
	};
}

#endif //_NM_SQUARESOURCE_H_
