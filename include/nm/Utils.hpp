#ifndef NM_UTILS_HPP
#define NM_UTILS_HPP

#include <execinfo.h>
#include <iomanip>
#include <iostream>
#include <nm/Chunk.hpp>
#include <sstream>

namespace nm
{
    namespace utils
    {
        /* An exception class that is thrown when we want to exit.
         * When using exit(), the stack isn't unwound - so lots of cleanup is not run.
         * This runs that cleanup.
         */
        class exit_unwind_stack
        {
        };

        template <typename T>
        std::wstring int_to_hex(T i)
        {
            std::wstringstream stream;
            stream << std::uppercase << std::setfill(L'0') << std::setw(sizeof(T) * 2) << std::hex
                   << i;
            return stream.str();
        }

        __attribute__((always_inline)) inline void print_backtrace(const int size)
        {
            void* objects[size];
            const int found = backtrace(objects, size);
            auto strings    = backtrace_symbols(objects, found);
            for (int i = 0; i < found; i++)
            {
                std::cout << strings[i] << std::endl;
            }
            free(static_cast<void*>(strings));
        }

        std::string ctsgdb(const char* s);
        const Coordinates to_chunk_coordinates(const Coordinates& c);
        const Coordinates to_global_coordinates(const Coordinates& local, const Coordinates& chunk);
        const Coordinates to_local_coordinates(const Coordinates& global);
        bool on_chunk_boundary(const Coordinates& local);
        void for_around(int x, int y, const std::function<void(int, int)>& functor);
    }  // namespace utils
}  // namespace nm

#endif  // NM_UTILS_HPP
