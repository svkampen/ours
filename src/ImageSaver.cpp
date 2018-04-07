#include <nm/ImageSaver.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

FT_Vector& operator+=(FT_Vector& rhs, FT_Vector lhs)
{
	rhs.x += lhs.x;
	rhs.y += lhs.y;
	return rhs;
}

namespace nm
{

	ImageSaver::ImageSaver(const ChunkSource& chunkSource) : chunkSource(chunkSource), face(library, "./berry.pcf.gz")
	{
		ft::set_face_size(face, 8);
	};

	void ImageSaver::write_block(uint32_t charcode, FT_Vector position, FT_Vector offset, png::rgb_pixel color, png::rgb_pixel blank_color)
	{
		ft::get_face_glyph_and_render(this->face, charcode, FT_RENDER_MODE_MONO);

		auto &bitmap = this->face->glyph->bitmap;
		auto&& buf = ft::expand_monochrome_bitmap(this->face);

		int x = position.x * glyph_size.x;
		int y = position.y * glyph_size.y;

		for (int dx = 0; dx < glyph_size.x; dx++)
		{
			for (int dy = 0; dy < glyph_size.y; dy++)
			{
				if (buf[dx + glyph_size.x * dy] == '0')
					this->image.set_pixel(x+dx+offset.x, y+dy+offset.y, blank_color);
				else
					this->image.set_pixel(x+dx+offset.x, y+dy+offset.y, color);
			}
		}
	}

	void ImageSaver::write_square(int x, int y, const Square& s)
	{
		png::rgb_pixel color;

		if (s.state == SquareState::CLOSED)
		{
			color = {0xEE, 0xEE, 0xEE};

			this->write_block(' ', {x, y}, {}, color, color);
			this->write_block(' ', {x-1, y}, {}, {0xEE, 0xEE, 0xEE}, {0xEE, 0xEE, 0xEE});
			this->write_block(' ', {x+1, y}, {}, {0xEE, 0xEE, 0xEE}, {0xEE, 0xEE, 0xEE});
		} else if (s.state == SquareState::FLAGGED)
		{
			color = {0x1E, 0x21, 0x2A};

			this->write_block(' ', {x+1, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
			this->write_block(' ', {x-1, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
			this->write_block('#', {x, y}, {}, {0xEE, 0xEE, 0xEE}, color);
		} else {
			switch (s.number)
			{
				case 1:
					color = {0x56, 0xB6, 0xC2}; break;
				case 2:
					color = {0x61, 0xAF, 0xEF}; break;
				case 3:
					color = {0x98, 0xC3, 0x79}; break;
				case 4:
					color = {0xC6, 0x78, 0xDD}; break;
				default:
					color = {0xE0, 0x6C, 0x75}; break;
			}

			if (s.number == 0)
			{
				this->write_block(' ', {x+1, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
				this->write_block(' ', {x-1, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
				this->write_block(' ', {x, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
			}
			else
			{
				char num_charcode = s.number + 48;
				this->write_block(' ', {x+1, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
				this->write_block(' ', {x-1, y}, {}, {0x1E, 0x21, 0x2A}, {0x1E, 0x21, 0x2A});
				this->write_block(num_charcode, {x, y}, {}, color, {0x1E, 0x21, 0x2A});
			}
		}
	}

	void ImageSaver::write_chunk(int x, int y, const Chunk& chunk = Chunk::CHUNK_EMPTY)
	{
		Coordinates begin({x * NM_CHUNK_SIZE, y * NM_CHUNK_SIZE});
		Coordinates end({(x + 1) * NM_CHUNK_SIZE, (y + 1)*NM_CHUNK_SIZE});

		for (int x = begin.x(); x < end.x(); x++)
		{
			for (int y = begin.y(); y < end.y(); ++y)
			{
				const Square& s = chunk.get({x - begin.x(), y - begin.y()});
				this->write_square(3*(x+1) - 2, y, s);
			}
		}
	}

	void ImageSaver::write_chunks(Coordinates begin, Coordinates end, const ChunkList& chunks)
	{
		for (int x = begin.x(); x < end.x(); x++)
		{
			for (int y = begin.y(); y < end.y(); y++)
			{
				// Subtract the starting point to normalize the range to [0, n)
				auto iter = chunks.find({x, y});
				if (iter == chunks.end())
					this->write_chunk(x - begin.x(), y - begin.y());
				else
					this->write_chunk(x - begin.x(), y - begin.y(), chunks.at({x, y}));
			}
		}
	}

	void ImageSaver::save(const std::string& filename)
	{
		auto& chunks = chunkSource.get_chunks();

		auto XFn = [](auto &a, auto &b) { return a.first.x() < b.first.x(); };
		auto YFn = [](auto &a, auto &b) { return a.first.y() < b.first.y(); };

		auto begin_x = std::min_element(chunks.cbegin(), chunks.cend(), XFn)->first.x();
		auto begin_y = std::min_element(chunks.cbegin(), chunks.cend(), YFn)->first.y();

		auto end_x = std::max_element(chunks.cbegin(), chunks.cend(), XFn)->first.x() + 1;
		auto end_y = std::max_element(chunks.cbegin(), chunks.cend(), YFn)->first.y() + 1;

		this->glyph_size = ft::get_bitmap_size(face, 'a');

		this->image = png::image<png::rgb_pixel>((end_x - begin_x) * NM_CHUNK_SIZE * glyph_size.x * 3,
				(end_y - begin_y) * NM_CHUNK_SIZE * glyph_size.y);

		this->write_chunks({begin_x, begin_y}, {end_x, end_y}, chunks);

		this->image.write(filename);
	}
}
