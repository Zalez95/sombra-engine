#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include <string>
#include "../utils/Image.h"

namespace loaders {

	/**
	 * ImageReader Class. Its used to read and load images from files.
	 */
	class ImageReader
	{
	public:		// Functions
		/** Creates a new ImageReader */
		ImageReader() {};

		/** Class destructor */
		~ImageReader() {};

		/** Reads the image located at the given position
		 * 
		 * @param	path the location of the image
		 * @param	imageFormat the format in which the image is stored */
		utils::Image* read(
			const std::string& path, utils::ImageFormat imageFormat
		) const;
	};

}

#endif		// IMAGE_READER_H
