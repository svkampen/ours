#include <nm/FreeType.hpp>

namespace nm
{
    namespace ft
    {
        signed error;

        Library::Library()
        {
            FT_Init_FreeType(&this->_l);
        }

        Library::~Library()
        {
            FT_Done_FreeType(this->_l);
        }

        Library::operator FT_Library&()
        {
            return this->_l;
        }
        Face::Face(FT_Library& lib, std::string path)
        {
            signed error;
            error = FT_New_Face(lib, path.c_str(), 0, &_f);
            if (error == FT_Err_Unknown_File_Format)
            {
                std::stringstream s;
                s << "The font you're trying to load has an unknown file "
                     "format. (Font: "
                  << path << ")." << std::endl;
                throw std::runtime_error(s.str());
            }
            else if (error)
            {
                std::stringstream s;
                s << "The font you're trying to load could not be opened or "
                     "read, or is broken. (Font: "
                  << path << ")." << std::endl;
                throw std::runtime_error(s.str());
            }
        }

        Face::~Face()
        {
            FT_Done_Face(_f);
        }

        Face::operator FT_Face&()
        {
            return _f;
        }

        FT_Face& Face::operator->()
        {
            return _f;
        }

        void set_face_size(FT_Face& face, int pt)
        {
            error = FT_Set_Char_Size(face, 0, pt * 64, 96, 96);
        }

        void get_face_glyph_and_render(FT_Face& face, uint32_t charcode, FT_Render_Mode render_mode)
        {
            signed glyph_index = FT_Get_Char_Index(face, charcode);
            error              = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

            if (error)
            {
                std::cerr << "Freetype error: " << error << std::endl;
            }

            error = FT_Render_Glyph(face->glyph, render_mode);
        }

        /* This function expands bitpacked monochrome bitmaps
         * to byte-per-pixel bitmaps (which wastes space, but is
         * much easier to handle in code.
         */
        std::string expand_monochrome_bitmap(FT_Face& face)
        {
            std::stringstream expanded_bitmap;

            auto& bitmap  = face->glyph->bitmap;
            auto& metrics = face->glyph->metrics;

            for (int row = 0; row < bitmap.rows; row++)
            {
                signed row_width_left = bitmap.width;
                for (int packed_pixels = 0; packed_pixels < bitmap.pitch && row_width_left > 0;
                     packed_pixels++, row_width_left -= 8)
                {
                    uint8_t data = bitmap.buffer[row * bitmap.pitch + packed_pixels];
                    if (row_width_left < 8)
                    {
                        for (int bit = 1; bit <= row_width_left; bit++)
                        {
                            if (data & (1 << (8 - bit)))
                                expanded_bitmap << '1';
                            else
                                expanded_bitmap << '0';
                        }
                    }
                    else
                    {
                        for (int bit = 0; bit < 8; bit++)
                        {
                            if (data & (1 << (7 - bit)))
                                expanded_bitmap << '1';
                            else
                                expanded_bitmap << '0';
                        }
                    }
                }
            }

            return expanded_bitmap.str();
        }

        /*
        if (face->glyph->metrics.horiBearingX > 64)
                row_string << std::string(face->glyph->metrics.horiBearingX /
        64, ' ');
        */

        template <typename T>
        decltype(auto) get_string(FT_Face& face, std::basic_string<T> str)
        {
            std::vector<std::string> rows(face->size->metrics.height / 64, "");
            auto origin = -face->size->metrics.descender / 64;
            std::cout << "origin: " << origin << std::endl;
            for (T& ch : str)
            {
                if (ch == ' ')
                {
                    for (auto& line : rows)
                    {
                        line += std::string(face->glyph->advance.x / 128, ' ');
                    }
                    continue;
                }
                std::vector<std::string> out = {};  //_output_character(face, ch);

                auto yMin = (face->glyph->metrics.height - face->glyph->metrics.horiBearingY) / 64;
                auto offset = origin - yMin;
                for (auto reverse_it = out.rbegin(), line_reverse_it = rows.rbegin();
                     line_reverse_it != rows.rend();
                     line_reverse_it++)
                {
                    if (reverse_it == out.rend() || offset)
                    {
                        *line_reverse_it += std::string(
                            face->glyph->bitmap.width + (face->glyph->metrics.horiBearingX / 64),
                            ' ');
                        offset--;
                    }
                    else
                    {
                        *line_reverse_it += *reverse_it;
                        reverse_it++;
                    }
                }
            }
            return rows;
        }

        FT_Vector get_bitmap_size(FT_Face& face, uint32_t charcode)
        {
            ft::get_face_glyph_and_render(face, charcode, FT_RENDER_MODE_MONO);
            return {face->glyph->bitmap.width, face->glyph->bitmap.rows};
        }
    }  // namespace ft
}  // namespace nm
