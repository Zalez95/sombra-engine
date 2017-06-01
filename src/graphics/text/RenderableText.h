#ifndef RENDERABLE_TEXT_H
#define RENDERABLE_TEXT_H

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Font.h"

namespace graphics {

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

		/** The scale of the Text Font */
		float mFontSize;

		/** The position of the Text */
		glm::vec2 mPosition;

	public:		// Functions
		/** Creates a new RenderableText
		 *
		 * @param	text the text to render of the RenderableText
		 * @param	font a pointer to the Font of the RenderableText
		 * @param	fontSize the scale of the RenderableText's Font
		 * @param	position the 2D position of the RenderableText */
		RenderableText(
			const std::string text, const FontSPtr font, float fontSize,
			const glm::vec2& position
		) :	mText(text), mFont(font), mFontSize(fontSize),
			mPosition(position) {};

		/** Class destructor */
		~RenderableText() {};

		/** @return	the string text of the RenderableText */
		inline std::string getText() const { return mText; };

		/** @return a pointer to the font of the RenderableText */
		inline FontSPtr getFont() const { return mFont; };

		/** @return the scale of the RenderableText's Font */
		inline float getFontSize() const { return mFontSize; };

		/** @return the position of the RenderableText */
		inline glm::vec2 getPosition() const { return mPosition; };
	};

}

#endif		// RENDERABLE_TEXT_H
