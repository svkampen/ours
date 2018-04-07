#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "Square.hpp"
#include "Typedefs.hpp"
#include "SquareSource.hpp"

#include <functional>
#include <memory>

#define NM_CHUNK_SIZE 16

namespace nm
{
	class Chunk : public SquareSource
	{
		using SquareFn = std::function<void(Square&)>;
		using ConstSquareFn = std::function<bool(const Square&)>;
		private:
			std::array<std::array<Square, NM_CHUNK_SIZE>, NM_CHUNK_SIZE> chunk{}; // {} necessary for 0-init
		public:
			static Chunk CHUNK_EMPTY;
			Square& get(const Coordinates& coordinates);
			Square& get(int x, int y);

			bool all_squares(const ConstSquareFn& functor) const;

			const Square& get(const Coordinates& coordinates) const;
			const Square& get(const int x, const int y) const;
			Chunk transform_copy(const SquareFn& functor) const;
			void transform(const SquareFn& functor);
			std::unique_ptr<char[]> serialize() const;
			void deserialize(const char* data);
	};
}

#endif // CHUNK_HPP
