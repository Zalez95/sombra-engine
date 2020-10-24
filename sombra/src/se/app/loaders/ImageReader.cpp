#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "se/app/loaders/ImageReader.h"

using namespace std::string_literals;

namespace se::app {

	Result ImageReader::read(const char* path, Image<unsigned char>& output, int forceNumChannels)
	{
		int width, height, channels;
		int stbForceChannels = (forceNumChannels <= 0)? STBI_default : forceNumChannels;
		unsigned char* pixels = stbi_load(path, &width, &height, &channels, stbForceChannels);
		if (!pixels) {
			return Result(false, "Error loading the image located in \""s + path + "\": " + stbi_failure_reason());
		}

		output = {
			std::unique_ptr<unsigned char>(pixels),
			static_cast<std::size_t>(width), static_cast<std::size_t>(height),
			channels
		};
		return Result();
	}


	Result ImageReader::readHDR(const char* path, Image<float>& output, int forceNumChannels)
	{
		int width, height, channels;
		int stbForceChannels = (forceNumChannels <= 0)? STBI_default : forceNumChannels;

		stbi_set_flip_vertically_on_load(true);
		float* pixels = stbi_loadf(path, &width, &height, &channels, stbForceChannels);
		stbi_set_flip_vertically_on_load(false);

		if (!pixels) {
			return Result(false, "Error loading the HDR image located in \""s + path + "\": " + stbi_failure_reason());
		}

		output = {
			std::unique_ptr<float>(pixels),
			static_cast<std::size_t>(width), static_cast<std::size_t>(height),
			channels
		};
		return Result();
	}

}
