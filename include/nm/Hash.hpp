#ifndef NM_HASH_HPP
#define NM_HASH_HPP

#include <cstring>

namespace nm
{
	namespace hash
	{
		class Hash
		{
			private:
			static const unsigned int FNV_PRIME = 16777619u;
			static const unsigned int OFFSET_BASIS = 2166136261u;

			int hash_value;

			static unsigned int hash(const char *str)
			{
				const size_t length = strlen(str) + 1;
				unsigned int hash = OFFSET_BASIS;
				for (size_t i = 0; i < length; ++i)
				{
					hash ^= *str++;
					hash *= FNV_PRIME;
				}
				return hash;
			}

			public:
			Hash(const char* str) {
				hash_value = hash(str);
			}

			operator unsigned int () const {
				return this->hash_value;
			}
		};
	}
}


#endif
