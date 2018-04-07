#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <boost/signals2.hpp>
#include <unordered_map>
#include <nm/PairHash.hpp>

const int BORDER_COLOR = 13;

namespace nm
{
	struct Coordinates : std::pair<int32_t, int32_t>
	{
		using std::pair<int32_t, int32_t>::pair;
		const int32_t& x() const { return this->first; }
		const int32_t& y() const { return this->second; }
		int32_t& x() { return this->first; }
		int32_t& y() { return this->second; }
	};

	struct CursorData
	{
		int32_t x;
		int32_t y;
		int32_t offset_x;
		int32_t offset_y;

		int32_t color;

		Coordinates to_global() const
		{
			return {x + offset_x, y + offset_y};
		}
	};	

	using SquareEvent  = boost::signals2::signal<void (int, int)>;
	using CursorMap	   = std::unordered_map<int32_t, CursorData>;
}

#endif // TYPEDEFS_HPP
