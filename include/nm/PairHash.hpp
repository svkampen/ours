#ifndef PAIRHASH_HPP
#define PAIRHASH_HPP

#include <cstdint>
#include <cstddef>

namespace nm
{
    std::size_t hash_combine(std::size_t left, std::size_t right);

    template <typename P>
    struct int_pair_hash
    {
        std::size_t operator()(P const& p) const
        {
            return hash_combine(p.first, p.second);
        }
    };
}  // namespace nm

#endif  // PAIRHASH_HPP
