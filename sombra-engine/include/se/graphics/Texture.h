#ifndef TEXTURE_H
#define TEXTURE_H

namespace se::graphics {

	/**
	 * The Color format of the Texture
	 * RGB		texture in color
	 * RGBA		texture in color with alpha
	 * L		texture in greyscale
	 * LA		texture in grayscale with alpha
	 */
	enum class TextureFormat
	{
		RED,
		GREEN,
		BLUE,
		ALPHA,
		RGB,
		RGBA
	};


	/**
	 * Defines the interpolation method used for mipmapping if enabled
	 */
	enum class TextureFilter
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
	enum class TextureWrap
	{
		REPEAT,
		CLAMP_TO_EDGE
	};


	/**
	 * Defines the data type of each component of the pixels of an Image
	 */
	enum class TexturePixelType
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
		/** The id of the texture object */
		unsigned int mTextureId;

		/** The filters used for minification and magnification (NEAREST by
		 * default) */
		TextureFilter mFilters[2];

		/** The behavior of the texture when it falls out of the [0.0, 1.0]
		 * range in each axis (REPEAT by default) */
		TextureWrap mWrappings[2];

	public:		// Functions
		/** Creates a new Texture */
		Texture();
		Texture(const Texture& other) = delete;
		Texture(Texture&& other);

		/** Class destructor */
		~Texture();

		/** Assignment operator */
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other);

		/** Sets the filtering method used by the texture
		 *
		 * @param	minification the method used in the minification process
		 * @param	magnification the method used in the magnification
		 *			process */
		void setFiltering(
			TextureFilter minification, TextureFilter magnification
		);

		/** Sets the behavior of the texture in each axis when a requested
		 * position falls out of the [0.0, 1.0] range.
		 *
		 * @param	x the TextureWrap in the x axis
		 * @param	y the TextureWrap in the y axis */
		void setWrapping(TextureWrap x, TextureWrap y);

		/** Sets the image data of the Texture
		 *
		 * @param	pixels the pixel data of the new Texture
		 * @param	type the type of the pixel data of the new Texture
		 * @param	format the TextureFormat of the new Texture
		 * @param	width the width of the new Texture
		 * @param	height the height of the new Texture */
		void setImage(
			const void* pixels, TexturePixelType type, TextureFormat format,
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
