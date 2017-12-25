#ifndef RENDERABLE_2D_H
#define RENDERABLE_2D_H

#include <memory>
#include <glm/glm.hpp>

namespace fe { namespace graphics {

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
		/** The position of the 2D element */
		glm::vec2 mPosition;

		/** The scale of the 2D element */
		glm::vec2 mScale;

		/** The texture of the 2D element */
		const TextureSPtr mTexture;

	public:		// Functions
		/** Creates a new Renderable2D
		 *
		 * @param	position the 2D position of the Renderable2D
		 * @param	scale the 2D scale of the Renderable2D
		 * @param	texture a pointer to the texture of the Renderable2D */
		Renderable2D(
			const glm::vec2& position, const glm::vec2& scale,
			const TextureSPtr texture
		) :	mPosition(position), mScale(scale), mTexture(texture) {};

		/** Class destructor */
		~Renderable2D() {};

		/** @return the position of the Renderable2D */
		inline glm::vec2 getPosition() const { return mPosition; };

		/** @return the scale of the Renderable2D */
		inline glm::vec2 getScale() const { return mScale; };

		/** @return the texture of the Renderable 2D */
		inline const TextureSPtr getTexture() const { return mTexture; };
	};

}}

#endif		// RENDERABLE_2D_H
