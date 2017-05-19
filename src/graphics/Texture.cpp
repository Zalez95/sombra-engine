#include "Texture.h"
#include <FreeImage.h>

namespace graphics {

	Texture::Texture(const std::string& texturePath, GLuint textureTarget) :
		mTexturePath(texturePath), mTextureTarget(textureTarget)
	{
		const char* path = texturePath.c_str();

		// The image format
		FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path, 0);
		if (format == FIF_UNKNOWN) {
			if ((format = FreeImage_GetFIFFromFilename(path)) == FIF_UNKNOWN) {
				return;
			}
		}

		// The bitmap of the image
		FIBITMAP* bitmap = FreeImage_Load(format, path);

		// Get the dimensions of the image
		int width = FreeImage_GetWidth(bitmap);
		int height = FreeImage_GetHeight(bitmap);

		// Get the pixels of the image RGB
		BYTE* pixels = FreeImage_GetBits(bitmap);

		// Create the texture from the image
		glGenTextures(1, &mTextureID);

		glBindTexture(mTextureTarget, mTextureID);	// Bind texture

		glTexImage2D(mTextureTarget, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(mTextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(mTextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindTexture(mTextureTarget, 0);			// Unbind texture

		FreeImage_Unload(bitmap);
	}


	Texture::~Texture()
	{
		glDeleteTextures(1, &mTextureID);
	}


	void Texture::bind() const
	{
		glBindTexture(mTextureTarget, mTextureID);
	}


	void Texture::unbind() const
	{
		glBindTexture(mTextureTarget, 0);
	}

}
