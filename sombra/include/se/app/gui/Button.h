#ifndef BUTTON_H
#define BUTTON_H

#include <functional>
#include "../../graphics/2D/RenderableSprite.h"
#include "../Repository.h"
#include "IComponent.h"
#include "IBounds.h"

namespace se::app {

	class Label;
	class GUIManager;


	/**
	 * Class Button, it's an IComponent used for doing actions when it's clicked
	 * with the mouse
	 */
	class Button : public IComponent
	{
	protected:	// Nested types
		using IBoundsUPtr = std::unique_ptr<IBounds>;

	private:	// Attributes
		/** A pointer to the GUIManager used for drawing the Button */
		GUIManager* mGUIManager;

		/** The bounds of the Button for checking if the mouse is over it */
		IBoundsUPtr mBounds;

		/** The RenderableSprite used for drawing of the Button */
		graphics::RenderableSprite mSprite;

		/** The Technique of the Button */
		Repository::ResourceRef<graphics::Technique> mTechnique;

		/** If the mouse is over or not */
		bool mIsOver;

		/** If the button is pressed or not */
		bool mIsPressed;

		/** The Label of the Button */
		Label* mLabel;

		/** The scale of the Label relative to the Button size */
		glm::vec2 mLabelScale;

		/** A pointer to the function to call when the Button is pressed */
		std::function<void()> mAction;

	public:		// Functions
		/** Creates a new Button
		 *
		 * @param	guiManager a pointer to the GUIManager used for drawing
		 *			the Button
		 * @param	bounds a pointer to the Bounds object used for checking if
		 *			the mouse is over the button or not */
		Button(GUIManager* guiManager, IBoundsUPtr bounds);
		Button(const Button& other);
		Button(Button&& other) = default;

		/** Class destructor */
		virtual ~Button();

		/** Assignment operator */
		Button& operator=(const Button& other);
		Button& operator=(Button&& other) = default;

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
		 * @param	labelScale the label scale relative to the Button one
		 * @note	the label is always located at the center of the Button and
		 *			its the next z-index */
		void setLabel(
			Label* label,
			const glm::vec2& labelScale = glm::vec2(1.0f)
		);

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
