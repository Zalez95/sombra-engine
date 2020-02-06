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
		using FontSPtr = std::shared_ptr<Font>;

	private:	// Attributes
		/** The position in pixels of the Text */
		glm::vec2 mPosition;

		/** The scale in pixels of each character in the Text */
		glm::vec2 mScale;

		/** The font of the Text */
		FontSPtr mFont;

		/** The RGBA color of the Text */
		glm::vec4 mColor;

		/** The text to render */
		std::string mText;

	public:		// Functions
		/** Creates a new RenderableText
		 *
		 * @param	position the 2D position in pixels of the RenderableText
		 * @param	scale the 2D scale in pixels of each character in the
		 *			RenderableText
		 * @param	font a pointer to the Font of the RenderableText
		 * @param	color the color of the RenderableText
		 * @param	text the text to render of the RenderableText */
		RenderableText(
			const glm::vec2& position, const glm::vec2& scale,
			FontSPtr font = nullptr,
			const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			const std::string text = ""
		) :	mPosition(position), mScale(scale),
			mFont(font), mColor(color), mText(text) {};

		/** @return	the position in pixels of the RenderableText */
		const glm::vec2& getPosition() const { return mPosition; };

		/** @return	the scale in pixels of each character in the
		 *			RenderableText */
		const glm::vec2& getScale() const { return mScale; };

		/** @return	a pointer to the font of the RenderableText */
		FontSPtr getFont() const { return mFont; };

		/** @return	the RGBA color of the RenderableText */
		const glm::vec4& getColor() const { return mColor; };

		/** @return	the string text of the RenderableText */
		const std::string& getText() const { return mText; };

		/** Sets the position of the RenderableText
		 *
		 * @param	position the new position in pixels of the RenderableText */
		void setPosition(const glm::vec2& position) { mPosition = position; };

		/** Sets the scale of the RenderableText
		 *
		 * @param	scale the new scale in pixels of the RenderableText */
		void setScale(const glm::vec2& scale) { mScale = scale; };

		/** Sets the font of the RenderableText
		 *
		 * @param	font a pointer to the Font of the RenderableText */
		void setFont(FontSPtr font) { mFont = font; };

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
