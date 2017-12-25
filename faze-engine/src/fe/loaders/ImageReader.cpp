#include "fe/loaders/ImageReader.h"
#include <stdexcept>
#include <SOIL/SOIL.h>

namespace fe { namespace loaders {

	utils::Image* ImageReader::read(const std::string& path, utils::ImageFormat imageFormat) const
	{
		// Load the image data with SOIL
		int soilFormat = imageFormat == (utils::ImageFormat::RGB_IMAGE)? SOIL_LOAD_RGB :
			imageFormat == (utils::ImageFormat::RGBA_IMAGE)? SOIL_LOAD_RGBA :
			imageFormat == (utils::ImageFormat::L_IMAGE)? SOIL_LOAD_L :
			SOIL_LOAD_LA;

		int width, height, channels;
		unsigned char* pixels = SOIL_load_image(path.c_str(), &width, &height, &channels, soilFormat);

		if (!pixels) {
			throw std::runtime_error(
				"Error loading the image located in \"" + path + "\": " +
				SOIL_last_result()
			);
		}

		return new utils::Image(pixels, width, height, channels, imageFormat);
	}

}}
