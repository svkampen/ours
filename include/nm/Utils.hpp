#ifndef NM_UTILS_HPP
#define NM_UTILS_HPP

#include <sstream>
#include <iomanip>
#include <nm/Chunk.hpp>

namespace nm
{
	namespace utils
	{
		template<typename T>
		std::wstring int_to_hex(T i)
		{
			std::wstringstream stream;
			stream << std::uppercase << std::setfill(L'0') << std::setw(sizeof(T) * 2)
				<< std::hex << i;
			return stream.str();
		}

		const Coordinates to_chunk_coordinates(const Coordinates& c);
		const Coordinates to_global_coordinates(const Coordinates& local, const Coordinates& chunk);
		void for_around(int x, int y, const std::function<void (int, int)>& functor);
	}
}

#endif //NM_UTILS_HPP
