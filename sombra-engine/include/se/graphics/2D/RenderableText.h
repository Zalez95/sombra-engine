#ifndef RENDERABLE_TEXT_H
#define RENDERABLE_TEXT_H

#include "Font.h"

namespace se::graphics {

	/**
	 * Class RenderableText, it's a 2D graphic entity used to render text.
	 * It that holds a position, scale and a Font
	 */
	class RenderableText
	{
	private:	// Nested types
		typedef std::shared_ptr<Font> FontSPtr;

	private:	// Attributes
		/** The text to render */
		std::string mText;

		/** The font of the text */
		const FontSPtr mFont;

		/** The position of the Text */
		glm::vec2 mPosition;

		/** The scale of the Text */
		glm::vec2 mScale;

	public:		// Functions
		/** Creates a new RenderableText
		 *
		 * @param	text the text to render of the RenderableText
		 * @param	font a pointer to the Font of the RenderableText
		 * @param	position the 2D position of the RenderableText
		 * @param	position the 2D scale of the RenderableText */
		RenderableText(
			const std::string text, const FontSPtr font,
			const glm::vec2& position, const glm::vec2& scale
		) :	mText(text), mFont(font), mPosition(position), mScale(scale) {};

		/** @return	the string text of the RenderableText */
		const std::string& getText() const { return mText; };

		/** @return a pointer to the font of the RenderableText */
		FontSPtr getFont() const { return mFont; };

		/** @return the position of the RenderableText */
		const glm::vec2& getPosition() const { return mPosition; };

		/** @return the scale of the RenderableText */
		const glm::vec2& getScale() const { return mScale; };
	};

}

#endif		// RENDERABLE_TEXT_H
