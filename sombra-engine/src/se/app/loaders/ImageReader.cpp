#include <SOIL/SOIL.h>
#include "se/app/loaders/ImageReader.h"

namespace se::app {

	Result ImageReader::read(const char* path, Image& output, int forceNumChannels)
	{
		using namespace std::string_literals;

		int soilForceChannels = forceNumChannels;
		if (forceNumChannels <= 0) {
			soilForceChannels = SOIL_LOAD_AUTO;
		}

		int width, height, channels;
		unsigned char* pixels = SOIL_load_image(path, &width, &height, &channels, soilForceChannels);
		if (!pixels) {
			return Result(false, "Error loading the image located in \""s + path + "\": " + SOIL_last_result());
		}

		output = Image{
			std::unique_ptr<std::byte>(reinterpret_cast<std::byte*>(pixels)),
			static_cast<std::size_t>(width), static_cast<std::size_t>(height),
			channels
		};
		return Result();
	}

}
