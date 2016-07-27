#ifndef NM_CLIENTDATA_HPP
#define NM_CLIENTDATA_HPP

#include <cstdint>

namespace nm
{
namespace server
{
	struct ClientData
	{
		int32_t x;
		int32_t y;
		int32_t id;
	};
}
}

#endif
