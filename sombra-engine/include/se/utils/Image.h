#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <cstddef>

namespace se::utils {

	/**
	 * The Color format of the Image
	 * RGB_IMAGE	image in color
	 * RGBA_IMAGE	image in color with alpha
	 * L_IMAGE		image in greyscale
	 * LA_IMAGE		image in grayscale with alpha
	 */
	enum class ImageFormat
	{
		RGB_IMAGE,
		RGBA_IMAGE,
		L_IMAGE,
		LA_IMAGE
	};


	/**
	 * Image Class. It holds the data of an image in memory.
	 */
	class Image
	{
	private:	// Attributes
		/** The pixel data of the image */
		std::unique_ptr<std::byte> mPixels;

		/** The width of the image in pixels */
		unsigned int mWidth;

		/** The height of the image in pixels */
		unsigned int mHeight;

		/** The number of channle of the image */
		unsigned int mChannels;

		/** The format of the image color */
		ImageFormat mImageFormat;

	public:		// Functions
		/** Creates a new Image from the given data
		 *
		 * @param	pixels a pointer to the pixel data of the new image
		 * @param	width the width in pixels of the new image
		 * @param	height the height in pixels of the new image
		 * @param	channels the number of channels of the new image
		 * @param	imageFormat the format of the new Image */
		Image(
			std::unique_ptr<std::byte> pixels,
			unsigned int width, unsigned int height,
			unsigned int channels, ImageFormat imageFormat
		) : mPixels(std::move(pixels)), mWidth(width), mHeight(height),
			mChannels(channels), mImageFormat(imageFormat) {};

		/** @return	a pointer to the pixel data of the image */
		inline std::byte* getPixels() const { return mPixels.get(); };

		/** @return	the width of the image in pixels */
		inline unsigned int getWidth() const { return mWidth; };

		/** @return	the height of the image in pixels */
		inline unsigned int getHeight() const { return mHeight; };

		/** @return	the number of channels of the image */
		inline unsigned int getChannels() const { return mChannels; };

		/** @return	the format of the pixels of the image */
		inline ImageFormat getImageFormat() const { return mImageFormat; };
	};

}

#endif		// IMAGE_H
