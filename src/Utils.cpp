#include <cmath>
#include <nm/Utils.hpp>

namespace nm
{
    namespace utils
    {
        constexpr int32_t modulo(const int32_t n, const int32_t mod)
        {
            int32_t temp = n % mod;
            if (temp < 0)
                return mod + temp;
            return temp;
        }

        bool on_chunk_boundary(const Coordinates& local)
        {
            return (local.x() == 0 || local.x() == 15 || local.y() == 0 || local.y() == 15);
        }

        const Coordinates to_local_coordinates(const Coordinates& global)
        {
            return {modulo(global.x(), NM_CHUNK_SIZE), modulo(global.y(), NM_CHUNK_SIZE)};
        }

        const Coordinates to_chunk_coordinates(const Coordinates& c)
        {
            int x = ((int) std::floor(c.x() / (double) NM_CHUNK_SIZE));
            int y = ((int) std::floor(c.y() / (double) NM_CHUNK_SIZE));

            return {x, y};
        }

        const Coordinates to_global_coordinates(const Coordinates& local, const Coordinates& chunk)
        {
            return {local.x() + (NM_CHUNK_SIZE * chunk.x()),
                    local.y() + (NM_CHUNK_SIZE * chunk.y())};
        }

        void for_around(int x, int y, const std::function<void(int, int)>& functor)
        {
            const int around_offsets[3] = {-1, 0, 1};

            for (auto&& xoff : around_offsets)
            {
                for (auto&& yoff : around_offsets)
                {
                    functor(x + xoff, y + yoff);
                }
            }
        }

        std::string ctsgdb(const char* s)
        {
            return std::string(s);
        }
    }  // namespace utils
}  // namespace nm
