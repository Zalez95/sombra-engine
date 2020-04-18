#ifndef RENDERABLE_TEXT_H
#define RENDERABLE_TEXT_H

#include "Font.h"

namespace se::graphics {

	/**
	 * Class RenderableText, it's a 2D graphic entity used to render text.
	 * It that holds its position, size and Font
	 */
	class RenderableText
	{
	private:	// Nested types
		using FontRef = Font::Repository::Reference;

	private:	// Attributes
		/** The position in pixels of the Text */
		glm::vec2 mPosition;

		/** The maximum size in pixels of each character in the Text
		 * @note	this value will be used for calculating the scale to apply
		 *			to each character depending on its own size in the Font */
		glm::vec2 mSize;

		/** The font of the Text */
		FontRef mFont;

		/** The RGBA color of the Text */
		glm::vec4 mColor;

		/** The text to render */
		std::string mText;

	public:		// Functions
		/** Creates a new RenderableText
		 *
		 * @param	position the 2D position in pixels of the RenderableText
		 * @param	size the 2D size in pixels of each character in the
		 *			RenderableText
		 * @param	font a pointer to the Font of the RenderableText
		 * @param	color the color of the RenderableText
		 * @param	text the text to render of the RenderableText */
		RenderableText(
			const glm::vec2& position, const glm::vec2& size,
			FontRef font = FontRef(),
			const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 1.0f },
			const std::string text = ""
		) : mPosition(position), mSize(size),
			mFont(font), mColor(color), mText(text) {};

		/** @return	the position in pixels of the RenderableText */
		const glm::vec2& getPosition() const { return mPosition; };

		/** @return	the maximum size in pixels of each character in the
		 *			RenderableText */
		const glm::vec2& getSize() const { return mSize; };

		/** @return	a pointer to the font of the RenderableText */
		FontRef getFont() const { return mFont; };

		/** @return	the RGBA color of the RenderableText */
		const glm::vec4& getColor() const { return mColor; };

		/** @return	the string text of the RenderableText */
		const std::string& getText() const { return mText; };

		/** Sets the position of the RenderableText
		 *
		 * @param	position the new position in pixels of the RenderableText */
		void setPosition(const glm::vec2& position) { mPosition = position; };

		/** Sets the maximum size of each of the characters in the Text
		 *
		 * @param	size the new maximum size in pixels of the characters in
		 *			the Text */
		void setSize(const glm::vec2& size) { mSize = size; };

		/** Sets the font of the RenderableText
		 *
		 * @param	font a Reference to the Font of the RenderableText */
		void setFont(FontRef font) { mFont = font; };

		/** Sets the color of the RenderableText
		 *
		 * @param	color the new RGBA color of the RenderableText */
		void setColor(const glm::vec4& color) { mColor = color; };

		/** Sets the text to render
		 *
		 * @param	text the new text to render */
		void setText(const std::string& text) { mText = text; };
	};

}

#endif		// RENDERABLE_TEXT_H
