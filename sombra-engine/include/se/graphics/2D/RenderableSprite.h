#ifndef RENDERABLE_SPRITE_H
#define RENDERABLE_SPRITE_H

#include <memory>
#include <glm/glm.hpp>
#include "Renderable2D.h"

namespace se::graphics {

	class Texture;


	/**
	 * Class RenderableText, it's a Renderable2D used for drawing
	 * textures or colored rectangles. It that holds its position,
	 * size and Texture
	 */
	class RenderableSprite : public Renderable2D
	{
	private:	// Nested types
		using TextureSPtr = std::shared_ptr<Texture>;

	private:	// Attributes
		/** The position in pixels of the RenderableSprite */
		glm::vec2 mPosition;

		/** The size in pixels of the RenderableSprite */
		glm::vec2 mSize;

		/** The RGBA color of the RenderableSprite */
		glm::vec4 mColor;

		/** The texture of the RenderableSprite */
		TextureSPtr mTexture;

	public:		// Functions
		/** Creates a new RenderableSprite
		 *
		 * @param	position the 2D position in pixels of the RenderableSprite
		 * @param	size the 2D size in pixels of the RenderableSprite
		 * @param	color the RGBA color of the RenderableSprite
		 * @param	texture a pointer to the texture of the RenderableSprite */
		RenderableSprite(
			const glm::vec2& position, const glm::vec2& size,
			const glm::vec4& color = glm::vec4(1.0f),
			const TextureSPtr texture = nullptr
		) :	mPosition(position), mSize(size),
			mColor(color), mTexture(texture) {};

		/** Sets the position of the RenderableSprite
		 *
		 * @param	position the new position in pixels of the
		 *			RenderableSprite */
		void setPosition(const glm::vec2& position) { mPosition = position; };

		/** Sets the size of the RenderableSprite
		 *
		 * @param	size the new size in pixels of the RenderableSprite */
		void setSize(const glm::vec2& size) { mSize = size; };

		/** Sets the color of the RenderableSprite
		 *
		 * @param	color the new RGBA color of the RenderableSprite */
		void setColor(const glm::vec4& color) { mColor = color; };

		/** Sets the texture of the RenderableSprite
		 *
		 * @param	texture the new texture of the RenderableSprite */
		void setTexture(TextureSPtr texture) { mTexture = texture; };

		/** Submits the vertices of the current RenderableSprite to the given
		 * Renderer2D
		 *
		 * @param	renderer the renderer where the RenderableSprite vertices
		 *			will be submitted */
		virtual void submitVertices(Renderer2D& renderer) const override;
	};

}

#endif		// RENDERABLE_SPRITE_H
