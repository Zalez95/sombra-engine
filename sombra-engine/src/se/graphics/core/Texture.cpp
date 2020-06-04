#include "se/graphics/core/Texture.h"
#include "GLWrapper.h"

namespace se::graphics {

	Texture::Texture(TextureType type) : mType(type), mSlot(0)
	{
		GL_WRAP( glGenTextures(1, &mTextureId) );
		SOMBRA_TRACE_LOG << "Created Texture " << mTextureId;

		setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);
	}


	Texture::Texture(Texture&& other) : mType(other.mType), mTextureId(other.mTextureId), mSlot(other.mSlot)
	{
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
			SOMBRA_TRACE_LOG << "Deleted Texture " << mTextureId;
		}

		mType = other.mType;
		mTextureId = other.mTextureId;
		mSlot = other.mSlot;
		other.mTextureId = 0;

		return *this;
	}


	void Texture::setFiltering(TextureFilter minification, TextureFilter magnification) const
	{
		int glMinFilter = toGLFilter(minification);
		int glMagFilter = toGLFilter(magnification);

		GL_WRAP( glBindTexture(toGLTexture(mType), mTextureId) );
		GL_WRAP( glTexParameteri(toGLTexture(mType), GL_TEXTURE_MIN_FILTER, glMinFilter) );
		GL_WRAP( glTexParameteri(toGLTexture(mType), GL_TEXTURE_MAG_FILTER, glMagFilter) );
	}


	void Texture::setWrapping(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR) const
	{
		int glWrapS = toGLWrap(wrapS);
		int glWrapT = toGLWrap(wrapT);
		int glWrapR = toGLWrap(wrapR);

		GL_WRAP( glBindTexture(toGLTexture(mType), mTextureId) );
		GL_WRAP( glTexParameteri(toGLTexture(mType), GL_TEXTURE_WRAP_S, glWrapS) );
		if (mType != TextureType::Texture1D) {
			GL_WRAP( glTexParameteri(toGLTexture(mType), GL_TEXTURE_WRAP_T, glWrapT) );
			if (mType != TextureType::Texture2D) {
				GL_WRAP( glTexParameteri(toGLTexture(mType), GL_TEXTURE_WRAP_R, glWrapR) );
			}
		}
	}


	void Texture::setImage(
		const void* pixels, TypeId type, ColorFormat format,
		std::size_t width, std::size_t height, std::size_t depth
	) const
	{
		GLenum glFormat = toGLColor(format);
		GLenum glType = toGLType(type);

		GL_WRAP( glBindTexture(toGLTexture(mType), mTextureId) );

		if (mType == TextureType::Texture1D) {
			GL_WRAP( glTexImage1D(
				toGLTexture(mType), 0,
				glFormat, static_cast<GLsizei>(width), 0,
				glFormat, glType, pixels
			) );
		}
		else if (mType == TextureType::Texture2D) {
			GL_WRAP( glTexImage2D(
				toGLTexture(mType), 0,
				glFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
				glFormat, glType, pixels
			) );
		}
		if (mType == TextureType::Texture3D) {
			GL_WRAP( glTexImage3D(
				toGLTexture(mType), 0,
				glFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), 0,
				glFormat, glType, pixels
			) );
		}
	}


	void Texture::generateMipMap() const
	{
		GL_WRAP( glBindTexture(toGLTexture(mType), mTextureId) );
		GL_WRAP( glGenerateMipmap(toGLTexture(mType)) );
	}


	void Texture::bind() const
	{
		GL_WRAP( glActiveTexture(GL_TEXTURE0 + mSlot) );
		GL_WRAP( glBindTexture(toGLTexture(mType), mTextureId) );
	}


	void Texture::unbind() const
	{
		GL_WRAP( glBindTexture(toGLTexture(mType), 0) );
	}

}
