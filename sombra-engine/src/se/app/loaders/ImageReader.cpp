#include <stb_image.h>
#include "se/app/loaders/ImageReader.h"

namespace se::app {

	Result ImageReader::read(const char* path, Image& output, int forceNumChannels)
	{
		using namespace std::string_literals;

		int stbForceChannels = forceNumChannels;
		if (forceNumChannels <= 0) {
			stbForceChannels = STBI_default;
		}

		int width, height, channels;
		unsigned char* pixels = stbi_load(path, &width, &height, &channels, stbForceChannels);
		if (!pixels) {
			return Result(false, "Error loading the image located in \""s + path + "\": " + stbi_failure_reason());
		}

		output = Image{
			std::unique_ptr<std::byte>(reinterpret_cast<std::byte*>(pixels)),
			static_cast<std::size_t>(width), static_cast<std::size_t>(height),
			channels
		};
		return Result();
	}

}
