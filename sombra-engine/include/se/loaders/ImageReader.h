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
		 * @param	imageFormat the format in which the image is stored
		 * @return	the new Image
		 * @throw	runtime_error if the image wasn't found */
		static utils::Image read(
			const std::string& path, utils::ImageFormat imageFormat
		);
	};

}

#endif		// IMAGE_READER_H
