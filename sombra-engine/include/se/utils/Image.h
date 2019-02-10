#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <cstddef>

namespace se::utils {

	/**
	 * The Color format of the Image
	 */
	enum class ImageFormat
	{
		RGB,		///< image in color
		RGBA,		///< image in color with alpha
		L,			///< image in greyscale
		LA			///< image in grayscale with alpha
	};


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

		/** The format of the image color */
		ImageFormat format;
	};

}

#endif		// IMAGE_H
