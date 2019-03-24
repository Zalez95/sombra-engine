#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <cstddef>

namespace se::utils {

	/**
	 * Struct Class, it holds the data of an image in memory.
	 */
	struct Image
	{
		/** The pixel data of the image */
		std::unique_ptr<std::byte> pixels;

		/** The width of the image in pixels */
		std::size_t width;

		/** The height of the image in pixels */
		std::size_t height;

		/** The number of channels of the image */
		int channels;
	};

}

#endif		// IMAGE_H
