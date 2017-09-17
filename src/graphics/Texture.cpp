#include "Texture.h"

namespace graphics {

	Texture::Texture() :
		mFilters{TextureFilter::NEAREST, TextureFilter::NEAREST},
		mWrappings{TextureWrap::REPEAT, TextureWrap::REPEAT}
	{
		glGenTextures(1, &mTextureID);
	}



	Texture::~Texture()
	{
		glDeleteTextures(1, &mTextureID);
	}


	void Texture::setImage(
		void* pixels, TexturePixelType type, TextureFormat format,
		unsigned int width, unsigned int height
	) {
		int glFormat = (format == TextureFormat::RGB)? GL_RGB :
						(format == TextureFormat::RGBA)? GL_RGBA :
						(format == TextureFormat::L)? GL_LUMINANCE :
						GL_LUMINANCE_ALPHA;
		int glType = (type == TexturePixelType::BYTE)? GL_BYTE :
						(type == TexturePixelType::U_BYTE)? GL_UNSIGNED_BYTE :
						(type == TexturePixelType::INT)? GL_INT :
						GL_FLOAT;

		int glFilters[2], glWrappings[2];
		for (size_t i = 0; i < 2; ++i) {
			glFilters[i] = (mFilters[i] == TextureFilter::NEAREST)? GL_NEAREST : GL_LINEAR;
			glWrappings[i] = (mWrappings[i] == TextureWrap::REPEAT)? GL_REPEAT : GL_CLAMP_TO_EDGE;
		}

		glBindTexture(GL_TEXTURE_2D, mTextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilters[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilters[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrappings[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrappings[1]);

		glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, glType, pixels);

		glBindTexture(GL_TEXTURE_2D, 0);
	}


	void Texture::bind(unsigned int slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, mTextureID);
	}


	void Texture::unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}
