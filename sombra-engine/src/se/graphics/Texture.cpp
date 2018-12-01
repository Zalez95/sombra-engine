#include "se/graphics/Texture.h"
#include "se/graphics/GLWrapper.h"

namespace se::graphics {

	Texture::Texture() :
		mFilters{TextureFilter::NEAREST, TextureFilter::NEAREST},
		mWrappings{TextureWrap::REPEAT, TextureWrap::REPEAT}
	{
		GL_WRAP( glGenTextures(1, &mTextureId) );
	}


	Texture::Texture(Texture&& other)
	{
		mTextureId = other.mTextureId;
		setFiltering(other.mFilters[0], other.mFilters[1]);
		setWrapping(other.mWrappings[0], other.mWrappings[1]);

		other.mTextureId = 0;
	}


	Texture::~Texture()
	{
		if (mTextureId != 0) {
			GL_WRAP( glDeleteTextures(1, &mTextureId) );
		}
	}


	Texture& Texture::operator=(Texture&& other)
	{
		if (mTextureId != 0) {
			GL_WRAP( glDeleteTextures(1, &mTextureId) );
		}

		mTextureId = other.mTextureId;
		setFiltering(other.mFilters[0], other.mFilters[1]);
		setWrapping(other.mWrappings[0], other.mWrappings[1]);

		other.mTextureId = 0;

		return *this;
	}


	void Texture::setFiltering(TextureFilter minification, TextureFilter magnification)
	{
		mFilters[0] = minification;
		mFilters[1] = magnification;
	}


	void Texture::setWrapping(TextureWrap x, TextureWrap y)
	{
		mWrappings[0] = x;
		mWrappings[1] = y;
	}


	void Texture::setImage(
		const void* pixels, TexturePixelType type, TextureFormat format,
		unsigned int width, unsigned int height
	) {
		int glFormat = (format == TextureFormat::RED)? GL_RED :
			(format == TextureFormat::GREEN)? GL_GREEN :
			(format == TextureFormat::BLUE)? GL_BLUE :
			(format == TextureFormat::ALPHA)? GL_ALPHA :
			(format == TextureFormat::RGB)? GL_RGB :
			GL_RGBA;
		int glType = (type == TexturePixelType::BYTE)? GL_BYTE :
			(type == TexturePixelType::U_BYTE)? GL_UNSIGNED_BYTE :
			(type == TexturePixelType::INT)? GL_INT :
			GL_FLOAT;

		int glFilters[2], glWrappings[2];
		for (std::size_t i = 0; i < 2; ++i) {
			glFilters[i] = (mFilters[i] == TextureFilter::NEAREST)? GL_NEAREST : GL_LINEAR;
			glWrappings[i] = (mWrappings[i] == TextureWrap::REPEAT)? GL_REPEAT : GL_CLAMP_TO_EDGE;
		}

		GL_WRAP( glBindTexture(GL_TEXTURE_2D, mTextureId) );

		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilters[0]) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilters[1]) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrappings[0]) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrappings[1]) );

		GL_WRAP( glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, glType, pixels) );

		GL_WRAP( glBindTexture(GL_TEXTURE_2D, 0) );
	}


	void Texture::bind(unsigned int slot) const
	{
		GL_WRAP( glActiveTexture(GL_TEXTURE0 + slot) );
		GL_WRAP( glBindTexture(GL_TEXTURE_2D, mTextureId) );
	}


	void Texture::unbind() const
	{
		GL_WRAP( glBindTexture(GL_TEXTURE_2D, 0) );
	}

}
