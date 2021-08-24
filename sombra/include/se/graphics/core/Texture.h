#ifndef TEXTURE_H
#define TEXTURE_H

#include "Bindable.h"
#include "Constants.h"

namespace se::graphics {

	/**
	 * Texture Class, it's a Bindable that hold data in an image format. It
	 * could be a 1D, 2D, 3D, 1D Array, 2D Array or CubeMap image.
	 * Texture can be accesed as variables from Shaders or as Render targets.
	 * @note	When the texture is a CubeMap, the Texture functions must use
	 *			an orientation parameter to operate with a specific side of the
	 *			cubemap.
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

		/** If the texture has multiple mipmap levels or not */
		bool mHasMipMaps;

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

		/** @return	the target of the Texture */
		inline TextureTarget getTarget() const { return mTarget; };

		/** @return	the id of the Texture */
		inline unsigned int getTextureId() const { return mTextureId; };

		/** @return	the current texture unit where the Texture will be bound */
		int getTextureUnit() const { return mTextureUnit; };

		/** Sets the texture unit where the texture will be bound
		 *
		 * @param	unit the texture unit where we want to bind the texture
		 * @return	a reference to the current Texture object */
		Texture& setTextureUnit(int unit);

		/** @return	the current image unit where the Texture will be bound */
		int getImageUnit() const { return mImageUnit; };

		/** Sets the image unit where the texture will be bound
		 *
		 * @param	unit the image unit where we want to bind the texture
		 * @return	a reference to the current Texture object */
		Texture& setImageUnit(int unit);

		/** Returns the filtering method used by the texture
		 *
		 * @param	minification a pointer to the method used in the
		 *			minification process (return parameter)
		 * @param	magnification a pointer to the method used in the
		 *			magnification process (return parameter)
		 * @return	a reference to the current Texture object */
		const Texture& getFiltering(
			TextureFilter* minification, TextureFilter* magnification
		) const;

		/** Sets the filtering method used by the texture
		 *
		 * @param	minification the method used in the minification process
		 * @param	magnification the method used in the magnification
		 *			process
		 * @return	a reference to the current Texture object */
		Texture& setFiltering(
			TextureFilter minification, TextureFilter magnification
		);

		/** Returns the behavior of the texture in each axis when a requested
		 * position falls out of the [0.0, 1.0] range.
		 *
		 * @param	wrapS a pointer to the the TextureWrap in the x axis
		 *			(return parameter)
		 * @param	wrapT a pointer to the the TextureWrap in the y axis
		 *			(return parameter)
		 * @param	wrapR a pointer to the the TextureWrap in the z axis
		 *			(return parameter)
		 * @return	a reference to the current Texture object */
		const Texture& getWrapping(
			TextureWrap* wrapS = nullptr,
			TextureWrap* wrapT = nullptr,
			TextureWrap* wrapR = nullptr
		) const;

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

		/** Returns the border color of the Texture
		 *
		 * @param	r a pointer to the red value (return parameter)
		 * @param	g a pointer to the green value (return parameter)
		 * @param	b a pointer to the blue value (return parameter)
		 * @param	a a pointer to the alpha value (return parameter)
		 * @param	orientation which face of the CubeMap is going to be checked
		 * @return	a reference to the current Texture object */
		const Texture& getBorderColor(
			float* r, float* g, float* b, float* a, int orientation = 0
		) const;

		/** Sets the border color of the Texture
		 *
		 * @param	r the red value of the new color
		 * @param	g the green value of the new color
		 * @param	b the blue value of the new color
		 * @param	a the alpha value of the new color
		 * @param	orientation which face of the CubeMap is going to be set
		 * @return	a reference to the current Texture object */
		Texture& setBorderColor(
			float r, float g, float b, float a, int orientation = 0
		);

		/** Returns the TypeId of all the components of a color
		 *
		 * @param	mipMapLevel the mip map level to check
		 * @param	orientation which face of the CubeMap is going to be checked
		 * @return	the TypeId */
		TypeId getTypeId(int mipMapLevel = 0, int orientation = 0) const;

		/** @return	the ColorFormat of the Texture */
		ColorFormat getColorFormat() const { return mColorFormat; };

		/** @return	true if the Texture has multiple mipmap levels, false
		 *			otherwise */
		bool hasMipMaps() const { return mHasMipMaps; };

		/** Returns the width of the Texture
		 *
		 * @param	mipMapLevel the mip map level to check
		 * @param	orientation which face of the CubeMap is going to be checked
		 * @return	the texture width */
		std::size_t getWidth(int mipMapLevel = 0, int orientation = 0) const;

		/** Returns the height of the Texture
		 *
		 * @param	mipMapLevel the mip map level to check
		 * @param	orientation which face of the CubeMap is going to be checked
		 * @return	the texture height */
		std::size_t getHeight(int mipMapLevel = 0, int orientation = 0) const;

		/** Returns the depth of the Texture
		 *
		 * @param	mipMapLevel the mip map level to check
		 * @param	orientation which face of the CubeMap is going to be checked
		 * @return	the texture depth */
		std::size_t getDepth(int mipMapLevel = 0, int orientation = 0) const;

		/** Returns the image data of the Texture
		 *
		 * @param	destinationType the type of the buffer data
		 * @param	destinationFormat the ColorFormat of the buffer data (it
		 *			must be unsized)
		 * @param	buffer a pointer to the buffer where the image data will
		 *			be stored
		 * @param	orientation which face of the CubeMap is going to be checked
		 * @return	a reference to the current Texture object */
		const Texture& getImage(
			TypeId destinationType, ColorFormat destinationFormat, void* buffer,
			int orientation = 0
		) const;

		/** Sets the image data of the Texture
		 *
		 * @param	source a pointer to the data to store in the Texture
		 * @param	sourceType the type of the source data
		 * @param	sourceFormat the ColorFormat of the source data (it must be
		 *			unsized)
		 * @param	textureFormat the ColorFormat of the texture
		 * @param	width the width of the new Texture
		 * @param	height the height of the new Texture (or number of layers)
		 * @param	depth the depth of the new Texture (or number of layers)
		 * @param	orientation which face of the CubeMap is going to be set
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

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override;

		/** Binds the Texture */
		virtual void bind() const override;

		/** Unbinds the Texture */
		virtual void unbind() const override;
	};

}

#endif		// TEXTURE_H
