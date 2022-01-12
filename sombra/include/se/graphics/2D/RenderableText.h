#ifndef RENDERABLE_TEXT_H
#define RENDERABLE_TEXT_H

#include "Font.h"
#include "Renderable2D.h"

namespace se::graphics {

	/**
	 * Class RenderableText, it's a Renderable2D used for rendering text.
	 * It that holds its position, size and Font
	 */
	class RenderableText : public Renderable2D
	{
	private:	// Nested types
		using FontSPtr = std::shared_ptr<Font>;

	private:	// Attributes
		/** The position in pixels of the RenderableText */
		glm::vec2 mPosition;

		/** The maximum size in pixels of each character in the RenderableText
		 * @note	this value will be used for calculating the scale to apply
		 *			to each character depending on its own size in the Font */
		glm::vec2 mSize;

		/** The font of the RenderableText */
		FontSPtr mFont;

		/** The RGBA color of the RenderableText */
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
			FontSPtr font = nullptr,
			const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 1.0f },
			const std::string text = ""
		) : mPosition(position), mSize(size),
			mFont(font), mColor(color), mText(text) {};

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
		void setFont(FontSPtr font) { mFont = font; };

		/** Sets the color of the RenderableText
		 *
		 * @param	color the new RGBA color of the RenderableText */
		void setColor(const glm::vec4& color) { mColor = color; };

		/** Sets the text to render
		 *
		 * @param	text the new text to render */
		void setText(const std::string& text) { mText = text; };

		/** @copydoc Renderable2D::submitVertices(Context::Query&, Renderer2D&) */
		virtual void submitVertices(
			Context::Query& q, Renderer2D& renderer
		) const override;
	};

}

#endif		// RENDERABLE_TEXT_H
