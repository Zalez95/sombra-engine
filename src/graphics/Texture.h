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

		/** Binds the Texture */
		void bind() const;

		/** Unbinds the Texture */
		void unbind() const;
	};

}

#endif		// TEXTURE_H
