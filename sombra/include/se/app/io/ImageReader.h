#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include "Result.h"
#include "../graphics/Image.h"

namespace se::app {

	/**
	 * ImageReader Class. Its used for reading and loading images from files.
	 */
	class ImageReader
	{
	public:		// Functions
		/** Reads the image located at the given path
		 *
		 * @param	path the location of the image
		 * @param	output where the new Image will be stored
		 * @param	forceNumChannels the number of channels of the image to
		 *			load (automatically detected by default)
		 * @return	a Result object with the result of the operation */
		static Result read(
			const char* path, Image<unsigned char>& output,
			int forceNumChannels = 0
		);

		/** Reads the HDR image located at the given path
		 *
		 * @param	path the location of the HDR image
		 * @param	output where the new Image will be stored
		 * @param	forceNumChannels the number of channels of the image to
		 *			load (automatically detected by default)
		 * @return	a Result object with the result of the operation */
		static Result readHDR(
			const char* path, Image<float>& output, int forceNumChannels = 0
		);
	};

}

#endif		// IMAGE_READER_H
