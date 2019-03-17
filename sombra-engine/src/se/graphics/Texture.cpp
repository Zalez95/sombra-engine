#include "se/graphics/Texture.h"
#include "se/graphics/GLWrapper.h"

namespace se::graphics {

	Texture::Texture()
	{
		GL_WRAP( glGenTextures(1, &mTextureId) );
		SOMBRA_TRACE_LOG << "Created Texture " << mTextureId;
	}


	Texture::Texture(Texture&& other)
	{
		mTextureId = other.mTextureId;
		other.mTextureId = 0;
	}


	Texture::~Texture()
	{
		if (mTextureId != 0) {
			GL_WRAP( glDeleteTextures(1, &mTextureId) );
			SOMBRA_TRACE_LOG << "Deleted Texture " << mTextureId;
		}
	}


	Texture& Texture::operator=(Texture&& other)
	{
		if (mTextureId != 0) {
			GL_WRAP( glDeleteTextures(1, &mTextureId) );
		}

		mTextureId = other.mTextureId;
		other.mTextureId = 0;

		return *this;
	}


	void Texture::setFiltering(TextureFilter minification, TextureFilter magnification)
	{
		int glMinFilter = toGLFilter(minification);
		int glMagFilter = toGLFilter(magnification);

		GL_WRAP( glBindTexture(GL_TEXTURE_2D, mTextureId) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMinFilter) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagFilter) );
		GL_WRAP( glBindTexture(GL_TEXTURE_2D, 0) );
	}


	void Texture::setWrapping(TextureWrap wrapS, TextureWrap wrapT)
	{
		int glWrapS = toGLWrap(wrapS);
		int glWrapT = toGLWrap(wrapT);

		GL_WRAP( glBindTexture(GL_TEXTURE_2D, mTextureId) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapS) );
		GL_WRAP( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapT) );
		GL_WRAP( glBindTexture(GL_TEXTURE_2D, 0) );
	}


	void Texture::setImage(
		const void* pixels, TypeId type, ColorFormat format,
		std::size_t width, std::size_t height
	) {
		GLenum glFormat = toGLColor(format);
		GLenum glType = toGLType(type);

		GL_WRAP( glBindTexture(GL_TEXTURE_2D, mTextureId) );
		GL_WRAP( glTexImage2D(
			GL_TEXTURE_2D, 0,
			glFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
			glFormat, glType, pixels
		) );
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
