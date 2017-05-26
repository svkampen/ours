#include <nm/PairHash.hpp>

namespace nm
{
	std::size_t hash_combine(std::size_t left, std::size_t right)
	{
		// Shamelessly stolen from boost::hash_combine.
		return left + 0x9e377b9 + (right << 6) + (right >> 2);
	}
	
}
