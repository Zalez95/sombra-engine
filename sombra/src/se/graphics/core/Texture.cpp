#include <cassert>
#include "se/graphics/core/Texture.h"
#include "GLWrapper.h"

namespace se::graphics {

	static constexpr int kMaxTextures = 16;		// 16 is the minimum number of textures for OpenGL 3.X
	static const Texture* sTextureUnits[kMaxTextures] = {};


	Texture::Texture(TextureTarget target) : mTarget(target), mTextureUnit(-1), mImageUnit(-1)
	{
		GL_WRAP( glGenTextures(1, &mTextureId) );
		SOMBRA_TRACE_LOG << "Created Texture " << mTextureId;

		setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);
	}


	Texture::Texture(Texture&& other) :
		mTarget(other.mTarget), mTextureId(other.mTextureId),
		mTextureUnit(other.mTextureUnit), mImageUnit(other.mImageUnit), mColorFormat(other.mColorFormat)
	{
		other.mTextureId = 0;
	}


	Texture::~Texture()
	{
		unbind();
		if (mTextureId != 0) {
			GL_WRAP( glDeleteTextures(1, &mTextureId) );
			SOMBRA_TRACE_LOG << "Deleted Texture " << mTextureId;
		}
	}


	Texture& Texture::operator=(Texture&& other)
	{
		unbind();
		if (mTextureId != 0) {
			GL_WRAP( glDeleteTextures(1, &mTextureId) );
			SOMBRA_TRACE_LOG << "Deleted Texture " << mTextureId;
		}

		mTarget = other.mTarget;
		mTextureId = other.mTextureId;
		mTextureUnit = other.mTextureUnit;
		mImageUnit = other.mImageUnit;
		mColorFormat = other.mColorFormat;
		other.mTextureId = 0;

		return *this;
	}


	Texture& Texture::setTextureUnit(int unit)
	{
		assert((unit >= 0) && (unit < kMaxTextures) && "Texture unit not valid");

		unbind();
		mTextureUnit = unit;
		return *this;
	}


	Texture& Texture::setImageUnit(int unit)
	{
		mImageUnit = unit;
		return *this;
	}


	Texture& Texture::setFiltering(TextureFilter minification, TextureFilter magnification)
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		int glMinFilter = toGLFilter(minification);
		int glMagFilter = toGLFilter(magnification);

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		GL_WRAP( glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, glMinFilter) );
		GL_WRAP( glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, glMagFilter) );

		return *this;
	}


	Texture& Texture::setWrapping(TextureWrap wrapS, TextureWrap wrapT, TextureWrap wrapR)
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		int glWrapS = toGLWrap(wrapS);
		int glWrapT = toGLWrap(wrapT);
		int glWrapR = toGLWrap(wrapR);

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		GL_WRAP( glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, glWrapS) );
		if (mTarget != TextureTarget::Texture1D) {
			GL_WRAP( glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, glWrapT) );
			if (mTarget != TextureTarget::Texture2D) {
				GL_WRAP( glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, glWrapR) );
			}
		}

		return *this;
	}


	Texture& Texture::setBorderColor(float r, float g, float b, float a)
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		float color[4] = { r, g, b, a };

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		GL_WRAP( glTexParameterfv(glTarget, GL_TEXTURE_BORDER_COLOR, color) );

		return *this;
	}


	Texture& Texture::setImage(
		const void* source, TypeId sourceType, ColorFormat sourceFormat,
		ColorFormat textureFormat,
		std::size_t width, std::size_t height, std::size_t depth, int orientation
	) {
		GLenum glTarget = toGLTextureTarget(mTarget);
		GLenum glType = toGLType(sourceType);
		GLenum glFormat = toGLColorFormat(sourceFormat);
		GLint glInternalFormat = toGLColorFormat(mColorFormat = textureFormat);

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		switch (mTarget) {
			case TextureTarget::Texture1D:
				GL_WRAP( glTexImage1D(
					glTarget, 0, glInternalFormat,
					static_cast<GLsizei>(width), 0,
					glFormat, glType, source
				) );
				break;
			case TextureTarget::Texture2D:
				GL_WRAP( glTexImage2D(
					glTarget, 0, glInternalFormat,
					static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
					glFormat, glType, source
				) );
				break;
			case TextureTarget::Texture3D:
				GL_WRAP( glTexImage3D(
					glTarget, 0, glInternalFormat,
					static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), 0,
					glFormat, glType, source
				) );
				break;
			case TextureTarget::CubeMap:
				GL_WRAP( glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation, 0, glInternalFormat,
					static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
					glFormat, glType, source
				) );
				break;
		}

		return *this;
	}


	Texture& Texture::generateMipMap()
	{
		GL_WRAP( glBindTexture(toGLTextureTarget(mTarget), mTextureId) );
		GL_WRAP( glGenerateMipmap(toGLTextureTarget(mTarget)) );

		return *this;
	}


	void Texture::bind() const
	{
		if (mTextureUnit >= 0) {
			GL_WRAP( glActiveTexture(GL_TEXTURE0 + mTextureUnit) );

			// There can't be two textures with different targets bound to the same texture unit
			if (sTextureUnits[mTextureUnit] && (sTextureUnits[mTextureUnit]->mTarget != mTarget)) {
				sTextureUnits[mTextureUnit]->unbind();
			}
			sTextureUnits[mTextureUnit] = this;
		}
		if (mImageUnit >= 0) {
			GL_WRAP( glBindImageTexture(mImageUnit, mTextureId, 0, GL_TRUE, 0, GL_READ_WRITE, toGLColorFormat(mColorFormat)); );
		}
		GL_WRAP( glBindTexture(toGLTextureTarget(mTarget), mTextureId) );
	}


	void Texture::unbind() const
	{
		if (sTextureUnits[mTextureUnit] == this) {
			sTextureUnits[mTextureUnit] = nullptr;
			GL_WRAP( glBindTexture(toGLTextureTarget(mTarget), 0) );
		}
	}

}
