#ifndef NM_IMAGESAVER_HPP
#define NM_IMAGESAVER_HPP

#include "ChunkSource.hpp"

#include <nm/FreeType.hpp>
#include <png++/png.hpp>

namespace nm
{
    class ImageSaver
    {
        const ChunkSource& chunkSource;
        png::image<png::rgb_pixel> image;

        ft::Library library;
        ft::Face face;

        FT_Vector glyph_size;

        void write_block(uint32_t charcode, FT_Vector position, FT_Vector offset,
                         png::rgb_pixel color, png::rgb_pixel blank_color);
        void write_chunks(Coordinates begin, Coordinates end, const ChunkList& chunks);
        void write_chunk(int x, int y, const Chunk& chunk);
        void write_square(int x, int y, const Square& s);

      public:
        ImageSaver(const ChunkSource& chunkSource);
        void save(const std::string& filename);
    };
}  // namespace nm

#endif  // NM_IMAGESAVER_HPP
