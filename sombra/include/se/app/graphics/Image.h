#ifndef IMAGE_H
#define IMAGE_H

#include <memory>

namespace se::app {

	/**
	 * Struct Class, it holds the data of an image in memory.
	 */
	template <typename T>
	struct Image
	{
		/** The pixel data of the image */
		std::unique_ptr<T[]> pixels;

		/** The width of the image in pixels */
		std::size_t width = 0;

		/** The height of the image in pixels */
		std::size_t height = 0;

		/** The number of channels of the image */
		int channels = 0;
	};


	template <typename T>
	Image<T> copy(const Image<T>& source)
	{
		Image<T> ret;

		std::size_t length = static_cast<std::size_t>(source.channels);
		length *= source.width * source.height;

		ret.pixels = std::unique_ptr<T[]>(new T[length]);
		ret.width = source.width;
		ret.height = source.height;
		ret.channels = source.channels;

		std::copy(
			source.pixels.get(), source.pixels.get() + length,
			ret.pixels.get()
		);

		return ret;
	}


}

#endif		// IMAGE_H
