#ifndef TEXTURE_H
#define TEXTURE_H

#include "Bindable.h"
#include "Constants.h"

namespace se::graphics {

	/**
	 * Texture Class, it's a Bindable that hold data in an image format. It
	 * could be a 1D, 2D or 3D image. Texture can be accesed as variables
	 * from Shaders or as Render targets.
	 */
	class Texture : public Bindable
	{
	private:	// Attributes
		/** The target of the Texture */
		TextureTarget mTarget;

		/** The id of the texture object */
		unsigned int mTextureId;

		/** The texture unit where the texture will be bound */
		int mTextureUnit;

		/** The image unit where the texture will be bound */
		int mImageUnit;

		/** The color format of the texture */
		ColorFormat mColorFormat;

	public:		// Functions
		/** Creates a new Texture
		 *
		 * @param	target the target of Texture to create
		 * @note	the generated Texture will have Nearest filtering enabled
		 *			by default */
		Texture(TextureTarget target);
		Texture(const Texture& other) = delete;
		Texture(Texture&& other);

		/** Class destructor */
		~Texture();

		/** Assignment operator */
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other);

		/** @return the target of the Texture */
		inline TextureTarget getTarget() const { return mTarget; };

		/** @return the id of the Texture */
		inline unsigned int getTextureId() const { return mTextureId; };

		/** Sets the texture unit where the texture will be bound
		 *
		 * @param	unit the texture unit where we want to bind the texture
		 * @return	a reference to the current Texture object */
		Texture& setTextureUnit(int unit);

		/** Sets the image unit where the texture will be bound
		 *
		 * @param	unit the image unit where we want to bind the texture
		 * @return	a reference to the current Texture object */
		Texture& setImageUnit(int unit);

		/** Sets the filtering method used by the texture
		 *
		 * @param	minification the method used in the minification process
		 * @param	magnification the method used in the magnification
		 *			process
		 * @return	a reference to the current Texture object */
		Texture& setFiltering(
			TextureFilter minification, TextureFilter magnification
		);

		/** Sets the behavior of the texture in each axis when a requested
		 * position falls out of the [0.0, 1.0] range.
		 *
		 * @param	wrapS the TextureWrap in the x axis
		 * @param	wrapT the TextureWrap in the y axis
		 * @param	wrapR the TextureWrap in the z axis
		 * @return	a reference to the current Texture object */
		Texture& setWrapping(
			TextureWrap wrapS = TextureWrap::Repeat,
			TextureWrap wrapT = TextureWrap::Repeat,
			TextureWrap wrapR = TextureWrap::Repeat
		);

		/** Sets the border color of the Texture
		 *
		 * @param	r the red value of the new color
		 * @param	g the green value of the new color
		 * @param	b the blue value of the new color
		 * @param	a the alpha value of the new color
		 * @return	a reference to the current Texture object */
		Texture& setBorderColor(float r, float g, float b, float a);

		/** Sets the image data of the Texture
		 *
		 * @param	source a pointer to the data to store in the Texture
		 * @param	sourceType the type of the source data
		 * @param	sourceFormat the ColorFormat of the source data
		 * @param	textureFormat the ColorFormat of the texture
		 * @param	width the width of the new Texture
		 * @param	height the height of the new Texture
		 * @param	depth the depth of the new Texture
		 * @param	orientation which face of the CubeMap is going to be set
		 *			(0 = positive X, 1 = negative X, 2 = positive Y,
		 *			3 = negative Y, 4 = positive Z, 5 = negative Z)
		 * @return	a reference to the current Texture object */
		Texture& setImage(
			const void* source, TypeId sourceType, ColorFormat sourceFormat,
			ColorFormat textureFormat,
			std::size_t width = 0, std::size_t height = 0,
			std::size_t depth = 0, int orientation = 0
		);

		/** Generate mipmaps for the current texture
		 *
		 * @return	a reference to the current Texture object */
		Texture& generateMipMap();

		/** Binds the Texture */
		void bind() const override;

		/** Unbinds the Texture */
		void unbind() const override;
	};

}

#endif		// TEXTURE_H
