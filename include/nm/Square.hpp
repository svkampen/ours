#ifndef SQUARE_HPP
#define SQUARE_HPP

#include <cstdint>

namespace nm
{
	enum class SquareState : uint8_t
	{
		CLOSED = 0, OPENED, FLAGGED
	};

	typedef struct __attribute__((packed))
	{
		SquareState state   : 3;
		bool		is_mine : 1;
		uint8_t	 	number  : 4;
	} Square;
}

#endif //SQUARE_HPP
