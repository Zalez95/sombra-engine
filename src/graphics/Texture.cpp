#include "Texture.h"
#include <stdexcept>
#include <SOIL.h>

namespace graphics {

	Texture::Texture(const std::string& texturePath, GLuint textureTarget) :
		mTexturePath(texturePath), mTextureTarget(textureTarget)
	{
		// Load the texture with SOIL
		mTextureID = SOIL_load_OGL_texture(
			mTexturePath.c_str(),
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			0
		);

		// Error checking
		if (mTextureID == 0) {
			throw std::runtime_error("Error loading the texture \"" + mTexturePath + "\".");
		}

		// Get the width and height of the texture
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &mWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &mHeight);

		// Unbind the texture
		unbind();
	}


	Texture::~Texture()
	{
		glDeleteTextures(1, &mTextureID);
	}


	void Texture::getPixels(GLfloat* data) const
	{
		bind();
		glGetTexImage(mTextureTarget, 0, GL_RGB, GL_FLOAT, data);
		unbind();
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
