#ifndef _NM_CURSORDATA_
#define _NM_CURSORDATA_

#include <cstdint>
#include <simmo/point.hpp>

namespace nm
{
	struct CursorData
	{
		int32_t x;
		int32_t y;
		int32_t offset_x;
		int32_t offset_y;

		int32_t color;

		simmo::point<int32_t, 2> to_global()
		{
			return {x + offset_x, y + offset_y};
		}
	};
}

#endif //_NM_CURSORDATA_
