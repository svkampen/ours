#ifndef _NM_CURSORDATA_
#define _NM_CURSORDATA_

#include <cstdint>

namespace nm
{
	struct CursorData
	{
		int32_t x;
		int32_t y;
		int32_t offset_x;
		int32_t offset_y;

		int32_t color;
	};
}

#endif //_NM_CURSORDATA_
