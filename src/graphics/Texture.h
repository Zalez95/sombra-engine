#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

namespace graphics {
	
	/**
	 * The Color format of the Texture
	 * RGB		texture in color
	 * RGBA		texture in color with alpha
	 * L		texture in greyscale
	 * LA		texture in grayscale with alpha
	 */
	enum TextureFormat
	{
		RGB,
		RGBA,
		L,
		LA
	};


	/**
	 * Defines the interpolation method used for mipmapping if enabled
	 */
	enum TextureFilter
	{
		NEAREST,
		LINEAR
	};


	/**
	 * Defines the behavior of the texture when a requested position falls
	 * out of the [0.0, 1.0] range.
	 * REPEAT			the texture repeats if it's out of the range
	 * CLAMP_TO_EDGE	the texture finishes at the last pixel of the texture
	 */
	enum TextureWrap
	{
		REPEAT,
		CLAMP_TO_EDGE
	};


	/**
	 * Defines the data type of each component of the pixels of an Image
	 */
	enum TexturePixelType
	{
		BYTE,
		U_BYTE,
		INT,
		FLOAT
	};


	/**
	 * Texture Class
	 */
	class Texture
	{
	private:	// Attributes
		/** The reference of the texture object */
		GLuint mTextureID;

		/** The filters used for minification and magnification (NEAREST by
		 * default) */
		TextureFilter mFilters[2];

		/** The behavior of the texture when it falls out of the [0.0, 1.0]
		 * range in each axis (REPEAT by default) */
		TextureWrap mWrappings[2];

	public:		// Functions
		/** Creates a new Texture */
		Texture();

		/** Class destructor */
		~Texture();

		/** Sets the filtering method used by the texture
		 *
		 * @param	minification the method used in the minification process
		 * @param	magnification the method used in the magnification
		 * 			process */
		inline void setFiltering(
			TextureFilter minification, TextureFilter magnification
		) { mFilters[0] = minification; mFilters[1] = magnification; };

		/** Sets the behavior of the texture in each axis when a requested
		 * position falls out of the [0.0, 1.0] range.
		 * 
		 * @param	x the TextureWrap in the x axis
		 * @param	y the TextureWrap in the y axis */
		inline void setWrapping(TextureWrap x, TextureWrap y)
		{ mWrappings[0] = x; mWrappings[1] = y; };

		/** Sets the image data of the Texture
		 *
		 * @param	pixels the pixel data of the new Texture
		 * @param	type the type of the pixel data of the new Texture
		 * @param	textureFormat the format of the new Texture
		 * @param	width the width of the new Texture
		 * @param	height the height of the new Texture */
		void setImage(
			void* pixels, TexturePixelType type, TextureFormat format,
			unsigned int width, unsigned int height
		);

		/** Binds the Texture
		 * 
		 * @param	slot the texture unit where we want to bind the texture */
		void bind(unsigned int slot = 0) const;

		/** Unbinds the Texture */
		void unbind() const;
	};

}

#endif		// TEXTURE_H
