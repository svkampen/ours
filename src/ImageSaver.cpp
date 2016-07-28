#include <nm/ImageSaver.hpp>

namespace nm
{

	ImageSaver::ImageSaver(ChunkSource& chunkSource) : chunkSource(chunkSource)
	{
	};


	void ImageSaver::save(const std::string& filename)
	{
		// Turn client mode on, since that means we don't need to think about
		// pesky chunk generation!
		chunkSource.set_client_mode(true);
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

		chunkSource.set_client_mode(false);
	}
}
