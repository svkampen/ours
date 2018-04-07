#ifndef NM_FREETYPE_HPP
#define NM_FREETYPE_HPP
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace nm
{
namespace ft
{
	class Library 
	{
		private:
			FT_Library _l;
		public:
			Library();
			~Library();
			operator FT_Library& ();
	};

	class Face
	{
		private:
			FT_Face _f;
		public:
			Face(FT_Library& lib, std::string path);
			~Face();
			operator FT_Face& ();
			FT_Face& operator->();
	};

	void set_face_size(FT_Face& face, int pt);

	void get_face_glyph_and_render(FT_Face& face, uint32_t charcode, FT_Render_Mode render_mode = FT_RENDER_MODE_MONO);

	/* This function expands bitpacked monochrome bitmaps
	 * to byte-per-pixel bitmaps (which wastes space, but is
	 * much easier to handle in code.
	 */
	std::string expand_monochrome_bitmap(FT_Face& face);

	/*
	if (face->glyph->metrics.horiBearingX > 64)
		row_string << std::string(face->glyph->metrics.horiBearingX / 64, ' ');
	*/

	template<typename T>
	decltype(auto) get_string(FT_Face& face, std::basic_string<T> str);

	FT_Vector get_bitmap_size(FT_Face& face, uint32_t charcode);
}
}

#endif
