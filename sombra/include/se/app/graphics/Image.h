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

}

#endif		// IMAGE_H
