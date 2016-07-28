#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "Square.hpp"
#include "Typedefs.hpp"

#include <functional>

#define NM_CHUNK_SIZE 32

namespace nm
{
	class Chunk
	{
		using SquareFn = std::function<void(Square&)>;
		private:
			std::array<std::array<Square, NM_CHUNK_SIZE>, NM_CHUNK_SIZE> chunk{}; // {} necessary for 0-init
		public:
			Square& get(const Coordinates& coordinates);
			Square& get(int x, int y);
			Chunk transform_copy(const SquareFn& functor) const;
			char *serialize() const;
			void deserialize(const char* data);
	};
}

#endif // CHUNK_HPP
