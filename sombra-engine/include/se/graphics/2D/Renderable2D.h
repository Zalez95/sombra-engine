#ifndef RENDERABLE_2D_H
#define RENDERABLE_2D_H

#include <memory>
#include <glm/glm.hpp>

namespace se::graphics {

	class Texture;


	/**
	 * Class Renderable2D, it's a 2D graphic entity that holds a position,
	 * scale and texture
	 */
	class Renderable2D
	{
	private:	// Nested types
		typedef std::shared_ptr<Texture> TextureSPtr;

	private:	// Attributes
		/** The position in pixels of the 2D element */
		glm::vec2 mPosition;

		/** The scale in pixels of the 2D element */
		glm::vec2 mScale;

		/** The RGBA color of the 2D element */
		glm::vec4 mColor;

		/** The texture of the 2D element */
		TextureSPtr mTexture;

	public:		// Functions
		/** Creates a new Renderable2D
		 *
		 * @param	position the 2D position in pixels of the Renderable2D
		 * @param	scale the 2D scale in pixels of the Renderable2D
		 * @param	color the RGBA color of the Renderable2D
		 * @param	texture a pointer to the texture of the Renderable2D */
		Renderable2D(
			const glm::vec2& position, const glm::vec2& scale,
			const glm::vec4& color, const TextureSPtr texture = nullptr
		) :	mPosition(position), mScale(scale),
			mColor(color), mTexture(texture) {};

		/** @return	the position in pixels of the Renderable2D */
		const glm::vec2& getPosition() const { return mPosition; };

		/** @return	the scale in pixels of the Renderable2D */
		const glm::vec2& getScale() const { return mScale; };

		/** @return	the RGBA color of the Renderable2D */
		const glm::vec4& getColor() const { return mColor; };

		/** @return	the texture of the Renderable 2D */
		TextureSPtr getTexture() const { return mTexture; };

		/** Sets the position of the Renderable2D
		 *
		 * @param	position the new position in pixels of the Renderable2D */
		void setPosition(const glm::vec2& position) { mPosition = position; };

		/** Sets the scale of the Renderable2D
		 *
		 * @param	scale the new scale in pixels of the Renderable2D */
		void setScale(const glm::vec2& scale) { mScale = scale; };

		/** Sets the color of the Renderable2D
		 *
		 * @param	color the new RGBA color of the Renderable2D */
		void setColor(const glm::vec4& color) { mColor = color; };

		/** Sets the texture of the Renderable2D
		 *
		 * @param	texture the new texture of the Renderable2D */
		void setTexture(TextureSPtr texture) { mTexture = texture; };
	};

}

#endif		// RENDERABLE_2D_H
