#include <cassert>
#include "se/graphics/core/Texture.h"
#include "GLWrapper.h"

namespace se::graphics {

	static constexpr int kMaxTextures = 16;		// 16 is the minimum number of textures for OpenGL 3.X
	static const Texture* sTextureUnits[kMaxTextures] = {};


	Texture::Texture(TextureTarget target) : mTarget(target), mTextureUnit(-1), mImageUnit(-1), mHasMipMaps(false)
	{
		GL_WRAP( glGenTextures(1, &mTextureId) );
		SOMBRA_TRACE_LOG << "Created Texture " << mTextureId;

		setFiltering(TextureFilter::Nearest, TextureFilter::Nearest);
	}


	Texture::Texture(Texture&& other) :
		mTarget(other.mTarget), mTextureId(other.mTextureId),
		mTextureUnit(other.mTextureUnit), mImageUnit(other.mImageUnit),
		mColorFormat(other.mColorFormat), mHasMipMaps(other.mHasMipMaps)
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
		mHasMipMaps = other.mHasMipMaps;
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


	const Texture& Texture::getFiltering(TextureFilter* minification, TextureFilter* magnification) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		int glMinFilter, glMagFilter;

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		GL_WRAP( glGetTexParameteriv(glTarget, GL_TEXTURE_MIN_FILTER, &glMinFilter) );
		GL_WRAP( glGetTexParameteriv(glTarget, GL_TEXTURE_MAG_FILTER, &glMagFilter) );

		*minification = fromGLFilter(glMinFilter);
		*magnification = fromGLFilter(glMagFilter);

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


	const Texture& Texture::getWrapping(TextureWrap* wrapS, TextureWrap* wrapT, TextureWrap* wrapR) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		int glWrapS, glWrapT, glWrapR;

		GL_WRAP( glBindTexture(glTarget, mTextureId) );

		GL_WRAP( glGetTexParameteriv(glTarget, GL_TEXTURE_WRAP_S, &glWrapS) );
		if (wrapS) {
			*wrapS = fromGLWrap(glWrapS);
		}

		if (mTarget != TextureTarget::Texture1D) {
			GL_WRAP( glGetTexParameteriv(glTarget, GL_TEXTURE_WRAP_T, &glWrapT) );
			if (wrapT) {
				*wrapT = fromGLWrap(glWrapT);
			}

			if (mTarget != TextureTarget::Texture2D) {
				GL_WRAP( glGetTexParameteriv(glTarget, GL_TEXTURE_WRAP_R, &glWrapR) );
				if (wrapR) {
					*wrapR = fromGLWrap(glWrapR);
				}
			}
		}

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


	const Texture& Texture::getBorderColor(float* r, float* g, float* b, float* a, int orientation) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);

		float color[4];
		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glGetTexParameterfv(GL_TEXTURE_BORDER_COLOR, GL_TEXTURE_BORDER_COLOR, color) );
		*r = color[0];
		*g = color[1];
		*b = color[2];
		*a = color[3];

		return *this;
	}


	Texture& Texture::setBorderColor(float r, float g, float b, float a, int orientation)
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		float color[4] = { r, g, b, a };

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glTexParameterfv(glTarget, GL_TEXTURE_BORDER_COLOR, color) );

		return *this;
	}


	TypeId Texture::getTypeId(int mipMapLevel, int orientation) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		GLint glType;

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glGetTexLevelParameteriv(glTarget, mipMapLevel, GL_TEXTURE_RED_TYPE, &glType) );

		return fromGLType(glType);
	}


	std::size_t Texture::getWidth(int mipMapLevel, int orientation) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		GLint glWidth;

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glGetTexLevelParameteriv(glTarget, mipMapLevel, GL_TEXTURE_WIDTH, &glWidth) );

		return glWidth;
	}


	std::size_t Texture::getHeight(int mipMapLevel, int orientation) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		GLint glHeight;

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glGetTexLevelParameteriv(glTarget, mipMapLevel, GL_TEXTURE_HEIGHT, &glHeight) );

		return glHeight;
	}


	std::size_t Texture::getDepth(int mipMapLevel, int orientation) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		GLint glDepth;

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glGetTexLevelParameteriv(glTarget, mipMapLevel, GL_TEXTURE_DEPTH, &glDepth) );

		return glDepth;
	}


	const Texture& Texture::getImage(TypeId destinationType, ColorFormat destinationFormat, void* buffer, int orientation) const
	{
		GLenum glTarget = toGLTextureTarget(mTarget);
		GLenum glType = toGLType(destinationType);
		GLenum glFormat = toGLColorFormat(destinationFormat);

		if ((destinationFormat == ColorFormat::RGB) || (destinationFormat == ColorFormat::RGB8)) {
			GL_WRAP( glPixelStorei(GL_PACK_ALIGNMENT, 1) );
		}

		GL_WRAP( glBindTexture(glTarget, mTextureId) );
		if (mTarget == TextureTarget::CubeMap) {
			glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation;
		}
		GL_WRAP( glGetTexImage(glTarget, 0, glFormat, glType, buffer) );

		if ((destinationFormat == ColorFormat::RGB) || (destinationFormat == ColorFormat::RGB8)) {
			GL_WRAP( glPixelStorei(GL_PACK_ALIGNMENT, 4) );
		}

		return *this;
	}


	Texture& Texture::setImage(
		const void* source, TypeId sourceType, ColorFormat sourceFormat,
		ColorFormat textureFormat,
		std::size_t width, std::size_t height, std::size_t depth, int orientation
	) {
		mHasMipMaps = false;

		GLenum glTarget = toGLTextureTarget(mTarget);
		GLenum glType = toGLType(sourceType);
		GLenum glFormat = toGLColorFormat(sourceFormat);
		GLint glInternalFormat = toGLColorFormat(mColorFormat = textureFormat);

		if (sourceFormat == ColorFormat::RGB) {
			GL_WRAP( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );
		}

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

		if (sourceFormat == ColorFormat::RGB) {
			GL_WRAP( glPixelStorei(GL_UNPACK_ALIGNMENT, 4) );
		}

		return *this;
	}


	Texture& Texture::generateMipMap()
	{
		GL_WRAP( glBindTexture(toGLTextureTarget(mTarget), mTextureId) );
		GL_WRAP( glGenerateMipmap(toGLTextureTarget(mTarget)) );
		mHasMipMaps = true;

		return *this;
	}


	std::unique_ptr<Bindable> Texture::clone() const
	{
		auto ret = std::make_unique<Texture>(mTarget);
		ret->mTextureUnit = mTextureUnit;
		ret->mImageUnit = mImageUnit;
		ret->mColorFormat = mColorFormat;

		std::size_t width = getWidth();
		std::size_t height = (mTarget != TextureTarget::Texture1D)? getHeight() : 1;
		std::size_t depth = ((mTarget != TextureTarget::Texture1D) && (mTarget != TextureTarget::Texture2D))? getDepth() : 1;
		TypeId type = getTypeId();

		if (mTarget == TextureTarget::CubeMap) {
			std::size_t sideSize = width * height * depth * toNumberOfComponents(mColorFormat) * toTypeSize(type);
			std::vector<std::byte> buffer(sideSize);
			for (int i = 0; i < 6; ++i) {
				getImage(type, toUnSizedColorFormat(mColorFormat), &buffer[i * sideSize], i);
				ret->setImage(buffer.data() + i * sideSize, type, toUnSizedColorFormat(mColorFormat), mColorFormat, width, height, 0, i);
			}
		}
		else {
			std::vector<std::byte> buffer(width * height * depth * toNumberOfComponents(mColorFormat) * toTypeSize(type));
			getImage(type, toUnSizedColorFormat(mColorFormat), buffer.data());
			ret->setImage(buffer.data(), type, toUnSizedColorFormat(mColorFormat), mColorFormat, width, height, depth);
		}

		TextureFilter min, mag;
		TextureWrap wrapS, wrapT, wrapR;
		getFiltering(&min, &mag);
		getWrapping(&wrapS, &wrapT, &wrapR);

		ret->setFiltering(min, mag)
			.setWrapping(wrapS, wrapT, wrapR);

		if (mHasMipMaps) {
			ret->generateMipMap();
		}

		return ret;
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
