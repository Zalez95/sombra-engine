#ifndef RENDERABLE_2D_H
#define RENDERABLE_2D_H

#include <glm/glm.hpp>
#include "../core/Texture.h"

namespace se::graphics {

	/**
	 * Class Renderable2D, it's a 2D graphic entity that holds a position,
	 * size and texture
	 */
	class Renderable2D
	{
	private:	// Nested types
		using TextureRef = Texture::Repository::Reference;

	private:	// Attributes
		/** The position in pixels of the 2D element */
		glm::vec2 mPosition;

		/** The size in pixels of the 2D element */
		glm::vec2 mSize;

		/** The RGBA color of the 2D element */
		glm::vec4 mColor;

		/** The texture of the 2D element */
		TextureRef mTexture;

	public:		// Functions
		/** Creates a new Renderable2D
		 *
		 * @param	position the 2D position in pixels of the Renderable2D
		 * @param	size the 2D size in pixels of the Renderable2D
		 * @param	color the RGBA color of the Renderable2D
		 * @param	texture a pointer to the texture of the Renderable2D */
		Renderable2D(
			const glm::vec2& position, const glm::vec2& size,
			const glm::vec4& color = glm::vec4(1.0f),
			const TextureRef texture = TextureRef()
		) :	mPosition(position), mSize(size),
			mColor(color), mTexture(texture) {};

		/** @return	the position in pixels of the Renderable2D */
		const glm::vec2& getPosition() const { return mPosition; };

		/** @return	the size in pixels of the Renderable2D */
		const glm::vec2& getSize() const { return mSize; };

		/** @return	the RGBA color of the Renderable2D */
		const glm::vec4& getColor() const { return mColor; };

		/** @return	the texture of the Renderable 2D */
		TextureRef getTexture() const { return mTexture; };

		/** Sets the position of the Renderable2D
		 *
		 * @param	position the new position in pixels of the Renderable2D */
		void setPosition(const glm::vec2& position) { mPosition = position; };

		/** Sets the size of the Renderable2D
		 *
		 * @param	size the new size in pixels of the Renderable2D */
		void setSize(const glm::vec2& size) { mSize = size; };

		/** Sets the color of the Renderable2D
		 *
		 * @param	color the new RGBA color of the Renderable2D */
		void setColor(const glm::vec4& color) { mColor = color; };

		/** Sets the texture of the Renderable2D
		 *
		 * @param	texture the new texture of the Renderable2D */
		void setTexture(TextureRef texture) { mTexture = texture; };
	};

}

#endif		// RENDERABLE_2D_H
