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
		private:
			std::array<std::array<Square, NM_CHUNK_SIZE>, NM_CHUNK_SIZE> chunk{}; // {} necessary for 0-init
		public:
			Square& get(Coordinates coordinates);
			Square& get(int x, int y);
			Chunk transform_copy(std::function<void(Square&)> functor);
			char *serialize();
			void deserialize(char* data);
	};
}

#endif // CHUNK_HPP
