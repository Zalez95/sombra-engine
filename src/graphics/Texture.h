#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <GL/glew.h>

namespace graphics {

	/**
	 * Texture Class
	 */
	class Texture
	{
	private:	// Attributes
		/** The path to the vertex shader file */
		const std::string mTexturePath;

		/** The reference of the texture object */
		GLuint mTextureID;

		/** The target to which the texture is bound */
		const GLuint mTextureTarget;

		/** The width of the texture */
		GLint mWidth;

		/** The height of the texture */
		GLint mHeight;

	public:		// Functions
		/** Creates a new Texture
		 * 
		 * @param	texturePath the path to the Texture
		 * @param	textureTarget the target to which the Texture is bound */
		Texture(const std::string& texturePath, GLuint textureTarget);

		/** Class destructor */
		~Texture();

		/** @return	the path of the Texture */
		inline std::string getTexturePath() const { return mTexturePath; };

		/** @return the width of the Texture */
		inline GLint getWidth() const { return mWidth; };

		/** @return the height of the Texture */
		inline GLint getHeight() const { return mHeight; };

		/** Gets the Texture's pixel data
		 * 
		 * @param	data a pointer to the array where we will store the
		 *			texture data */
		void getPixels(GLfloat* data) const;

		/** Binds the Texture */
		void bind() const;

		/** Unbinds the Texture */
		void unbind() const;
	};

}

#endif		// TEXTURE_H
