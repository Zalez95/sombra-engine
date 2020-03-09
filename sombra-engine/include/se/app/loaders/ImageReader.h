#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include "Result.h"
#include "../Image.h"

namespace se::app {

	/**
	 * ImageReader Class. Its used to read and load images from files.
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
			const char* path, Image& output, int forceNumChannels = 0
		);
	};

}

#endif		// IMAGE_READER_H
