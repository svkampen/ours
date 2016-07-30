#include <nm/ImageSaver.hpp>

namespace nm
{

	ImageSaver::ImageSaver(ChunkSource& chunkSource) : chunkSource(chunkSource)
	{
	};

	void ImageSaver::write_chunk(int x, int y, const Chunk& chunk = Chunk::CHUNK_EMPTY)
	{
		Coordinates begin({x * NM_CHUNK_SIZE, y * NM_CHUNK_SIZE});
		Coordinates end({(x + 1) * NM_CHUNK_SIZE, (y + 1)*NM_CHUNK_SIZE});

		for (int x = begin.x(); x < end.x(); x++)
		{
			for (int y = begin.y(); y < end.y(); ++y)
			{
				png::rgb_pixel color;
				const Square& s = chunk.get({x - begin.x(), y - begin.y()});
				if (s.state == SquareState::CLOSED)
				{
					color = {255, 255, 255};
				} else if (s.state == SquareState::FLAGGED)
				{
					color = {0, 0, 0};
				} else {
					switch (s.number)
					{
						case 0:
							color = {128, 128, 128}; break;
						case 1:
							color = {116, 164, 187}; break;
						case 2:
							color = {88, 117, 175}; break;
						case 3:
							color = {94, 171, 122}; break;
						case 4:
							color = {143, 123, 192}; break;
						default:
							color = {255, 95, 103}; break;
					}
				}

				this->image.set_pixel(x, y, color);
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

		auto end_x = std::max_element(chunks.cbegin(), chunks.cend(), XFn)->first.x();
		auto end_y = std::max_element(chunks.cbegin(), chunks.cend(), YFn)->first.y();

		this->image = png::image<png::rgb_pixel>((end_x - begin_x) * NM_CHUNK_SIZE,
				(end_y - begin_y) * NM_CHUNK_SIZE);

		this->write_chunks({begin_x, begin_y}, {end_x, end_y}, chunks);

		this->image.write(filename);
	}
}
