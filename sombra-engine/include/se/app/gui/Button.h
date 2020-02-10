#ifndef BUTTON_H
#define BUTTON_H

#include <memory>
#include <functional>
#include "IComponent.h"
#include "IBounds.h"
#include "../../graphics/2D/Renderable2D.h"
#include "../../graphics/2D/Layer2D.h"

namespace se::app {

	class Label;


	/**
	 * Class Button, it's an IComponent used for doing actions when it's clicked
	 * with the mouse
	 */
	class Button : public IComponent
	{
	protected:	// Nested types
		using IBoundsIPtr = std::unique_ptr<IBounds>;

	private:	// Attributes
		/** A pointer to the Layer2D where @see Renderable2D will be submitted
		 * for drawing the Button */
		graphics::Layer2D* mLayer2D;

		/** The bounds of the Button for checking if the mouse is over it */
		IBoundsIPtr mBounds;

		/** The Renderable2D used for drawing of the Button */
		graphics::Renderable2D mRenderable2D;

		/** If the mouse is over or not */
		bool mIsOver;

		/** If the button is pressed or not */
		bool mIsPressed;

		/** The Label of the Button */
		Label* mLabel;

		/** A pointer to the function to call when the Button is pressed */
		std::function<void()> mAction;

	public:		// Functions
		/** Creates a new Button
		 *
		 * @param	layer2D a pointer to the Layer2D where the button will be
		 *			drawn
		 * @param	bounds a pointer to the Bounds object used for checking if
		 *			the mouse is over the button or not */
		Button(graphics::Layer2D* layer2D, IBoundsIPtr bounds);

		/** Class destructor */
		virtual ~Button();

		/** Sets the position of the Button
		 *
		 * @param	position the new Position of the top-left corner of the
		 *			Button */
		virtual void setPosition(const glm::vec2& position) override;

		/** Sets the size of the Button
		 *
		 * @param	size the new Size of the Button */
		virtual void setSize(const glm::vec2& size) override;

		/** Sets the z-index of the Button
		 *
		 * @param	zIndex the new z-index of the Button */
		virtual void setZIndex(unsigned char zIndex) override;

		/** Sets the Button visibility on/off
		 *
		 * @param	isVisible if Button must be shown or not */
		virtual void setVisibility(bool isVisible) override;

		/** Sets the color of the Button
		 *
		 * @param	color the new color of the Button */
		void setColor(const glm::vec4& color);

		/** Sets the label of the Button
		 *
		 * @param	label a pointer to the ne Label of the Button
		 * @note	the label is always located at the center of the Button and
		 *			its the next z-index */
		void setLabel(Label* label);

		/** Performs the given action when the Button is pressed
		 *
		 * @param	action the function to execute */
		void setAction(const std::function<void()>& action);

		/** Handles a mouse pointer over the Button
		 *
		 * @param	event the MouseMoveEvent that holds the location of the
		 *			mouse */
		virtual void onHover(const MouseMoveEvent& event) override;

		/** Handles a mouse click on the Button
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onClick(const MouseButtonEvent& event) override;

		/** Handles a mouse click release on the Button
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onRelease(const MouseButtonEvent& event) override;
	};

}

#endif		// BUTTON_H
