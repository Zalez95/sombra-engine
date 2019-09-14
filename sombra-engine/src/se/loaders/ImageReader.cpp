#include <SOIL/SOIL.h>
#include "se/loaders/ImageReader.h"

namespace se::loaders {

	Result ImageReader::read(const std::string& path, utils::Image& output, int forceNumChannels)
	{
		int soilForceChannels = forceNumChannels;
		if (forceNumChannels <= 0) {
			soilForceChannels = SOIL_LOAD_AUTO;
		}

		int width, height, channels;
		unsigned char* pixels = SOIL_load_image(path.c_str(), &width, &height, &channels, soilForceChannels);
		if (!pixels) {
			return Result(false, "Error loading the image located in \"" + path + "\": " + SOIL_last_result());
		}

		output = utils::Image{
			std::unique_ptr<std::byte>(reinterpret_cast<std::byte*>(pixels)),
			static_cast<std::size_t>(width), static_cast<std::size_t>(height),
			channels
		};
		return Result();
	}

}
