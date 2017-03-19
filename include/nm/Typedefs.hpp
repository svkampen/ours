#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <boost/signals2.hpp>
#include <simmo/point.hpp>
#include <nm/CursorData.hpp>
#include <unordered_map>

const int BORDER_COLOR = 15;

namespace nm
{
	using Coordinates  = simmo::point<int32_t, 2>;
	using SquareEvent  = boost::signals2::signal<void (int, int)>;
	using CursorMap	   = std::unordered_map<int32_t, CursorData>;
}

#endif // TYPEDEFS_HPP
