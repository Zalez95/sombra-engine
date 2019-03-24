#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include <string>
#include "../utils/Image.h"

namespace se::loaders {

	/**
	 * ImageReader Class. Its used to read and load images from files.
	 */
	class ImageReader
	{
	public:		// Functions
		/** Reads the image located at the given position
		 *
		 * @param	path the location of the image
		 * @param	forceNumChannels the number of channels of the image to
		 *			load (automatically detected by default)
		 * @return	the new Image
		 * @throw	runtime_error if there were any errors while reading the
		 *			image */
		static utils::Image read(
			const std::string& path, int forceNumChannels = 0
		);
	};

}

#endif		// IMAGE_READER_H
