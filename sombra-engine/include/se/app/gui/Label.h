#ifndef LABEL_H
#define LABEL_H

#include "IComponent.h"
#include "../../graphics/2D/RenderableText.h"
#include "../../graphics/2D/Layer2D.h"

namespace se::app {

	/**
	 * Class Label, it's an IComponent used for drawing the texts of the GUI
	 */
	class Label : public IComponent
	{
	private:	// Nested types
		using FontSPtr = std::shared_ptr<graphics::Font>;

	private:	// Attributes
		/** The RenderableText used for drawing of the Label */
		graphics::RenderableText mRenderableText;

		/** A pointer to the Layer2D where @see RenderableText will be submitted
		 * for drawing the Label */
		graphics::Layer2D* mLayer2D;

	public:		// Functions
		/** Creates a new Label
		 *
		 * @param	layer2D a pointer to the Layer2D where the Label will be
		 *			drawn */
		Label(graphics::Layer2D* layer2D);

		/** Class destructor */
		virtual ~Label();

		/** Sets the position of the Label
		 *
		 * @param	position the new Position of the top-left corner of the
		 *			Label */
		virtual void setPosition(const glm::vec2& position) override;

		/** Sets the size of the Label
		 *
		 * @param	size the new Size of the Label */
		virtual void setSize(const glm::vec2& size) override;

		/** Sets the z-index of the Label
		 *
		 * @param	zIndex the new z-index of the Label */
		virtual void setZIndex(unsigned char zIndex) override;

		/** Sets the Label visibility on/off
		 *
		 * @param	isVisible if Label must be shown or not */
		virtual void setVisibility(bool isVisible) override;

		/** Sets the font of the Label
		 *
		 * @param	font a pointer to the Font of the Label */
		void setFont(FontSPtr font);

		/** Sets the scale of each character in the Label text
		 *
		 * @param	scale the new scale in pixels of the Label text */
		void setCharacterScale(const glm::vec2& scale);

		/** Sets the color of the Label
		 *
		 * @param	color the new RGBA color of the Label */
		void setColor(const glm::vec4& color);

		/** Sets the text of the Label
		 *
		 * @param	text the new text of the Label */
		void setText(const std::string& text);

		/** Handles a mouse pointer over the Label
		 *
		 * @param	event the MouseMoveEvent that holds the location of the
		 *			mouse */
		virtual void onHover(const MouseMoveEvent& event) override;

		/** Handles a mouse click on the Label
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onClick(const MouseButtonEvent& event) override;

		/** Handles a mouse click release on the Label
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onRelease(const MouseButtonEvent& event) override;
	};

}

#endif		// LABEL_H
