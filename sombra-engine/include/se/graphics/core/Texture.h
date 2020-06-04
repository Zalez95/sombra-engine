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
		/** The type of the Texture */
		TextureType mType;

		/** The id of the texture object */
		unsigned int mTextureId;

		/** The texture unit where the texture will be bound */
		unsigned int mSlot;

	public:		// Functions
		/** Creates a new Texture
		 *
		 * @param	type the Type of Texture to create
		 * @note	the generated Texture will have Nearest filtering enabled
		 *			by default */
		Texture(TextureType type);
		Texture(const Texture& other) = delete;
		Texture(Texture&& other);

		/** Class destructor */
		~Texture();

		/** Assignment operator */
		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other);

		/** @return the id of the Texture */
		inline unsigned int getTextureId() const { return mTextureId; };

		/** Sets the texture unit where the texture will be bound
		 *
		 * @param	slot the texture unit where we want to bind the texture */
		void setTextureUnit(unsigned int slot) { mSlot = slot; };

		/** Sets the filtering method used by the texture
		 *
		 * @param	minification the method used in the minification process
		 * @param	magnification the method used in the magnification
		 *			process */
		void setFiltering(
			TextureFilter minification, TextureFilter magnification
		) const;

		/** Sets the behavior of the texture in each axis when a requested
		 * position falls out of the [0.0, 1.0] range.
		 *
		 * @param	wrapS the TextureWrap in the x axis
		 * @param	wrapT the TextureWrap in the y axis
		 * @param	wrapR the TextureWrap in the z axis */
		void setWrapping(
			TextureWrap wrapS = TextureWrap::Repeat,
			TextureWrap wrapT = TextureWrap::Repeat,
			TextureWrap wrapR = TextureWrap::Repeat
		) const;

		/** Sets the image data of the Texture
		 *
		 * @param	pixels the pixel data of the new Texture
		 * @param	type the type of the pixel data of the new Texture
		 * @param	format the ColorFormat of the new Texture
		 * @param	width the width of the new Texture
		 * @param	height the height of the new Texture
		 * @param	depth the depth of the new Texture */
		void setImage(
			const void* pixels, TypeId type, ColorFormat format,
			std::size_t width = 0, std::size_t height = 0, std::size_t depth = 0
		) const;

		/** Generate mipmaps for the current texture */
		void generateMipMap() const;

		/** Binds the Texture */
		void bind() const override;

		/** Unbinds the Texture */
		void unbind() const override;
	};

}

#endif		// TEXTURE_H
