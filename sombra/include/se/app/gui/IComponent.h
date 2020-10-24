#ifndef I_COMPONENT_H
#define I_COMPONENT_H

#include <glm/glm.hpp>
#include "../events/MouseEvent.h"

namespace se::app {

	/**
	 * Class IComponent, it's an interface that each GUI element must implement
	 * to be rendered or notified upon the user input
	 */
	class IComponent
	{
	protected:	// Attributes
		/** The IComponent dimensions in pixels in the X and Y axis */
		glm::vec2 mSize;

		/** The location of the top-left corner of the IComponent in pixels
		 * in the X and Y axis */
		glm::vec2 mPosition;

		/** The z-index used for drawing the IComponent on top of other
		 * IComponents. The smaller the value the further back it will
		 * appear. */
		unsigned char mZIndex;

		/** If the IComponent is visible or not */
		bool mIsVisible;

	public:		// Functions
		/** Creates a new IComponent */
		IComponent() :
			mSize(0.0f), mPosition(0.0f), mZIndex(0), mIsVisible(true) {};
		IComponent(const IComponent& other) = default;
		IComponent(IComponent&& other) = default;

		/** Class destructor */
		virtual ~IComponent() = default;

		/** Assignment operator */
		IComponent& operator=(const IComponent& other) = default;
		IComponent& operator=(IComponent&& other) = default;

		/** @return	the IComponent dimensions in the X and Y axis */
		const glm::vec2& getSize() const { return mSize; };

		/** @return	the location of the top-left corner of the IComponent in
		 *			the X and Y axis */
		const glm::vec2& getPosition() const { return mPosition; };

		/** @return	the z-index of the IComponent */
		unsigned char getZIndex() const { return mZIndex; };

		/** @return	true if the IComponent is visible, false otherwise */
		bool isVisible() const { return mIsVisible; };

		/** Sets the position of the IComponent
		 *
		 * @param	position the new Position of the top-left corner of the
		 *			IComponent */
		virtual void setPosition(const glm::vec2& position)
		{ mPosition = position; };

		/** Sets the size of the IComponent
		 *
		 * @param	size the new Size of the IComponent */
		virtual void setSize(const glm::vec2& size) { mSize = size; };

		/** Sets the z-index of the IComponent
		 *
		 * @param	zIndex the new z-index of the IComponent */
		virtual void setZIndex(unsigned char zIndex) { mZIndex = zIndex; };

		/** Sets the IComponent visibility on/off
		 *
		 * @param	isVisible if IComponent must be shown or not */
		virtual void setVisibility(bool isVisible) { mIsVisible = isVisible; };

		/** Handles a mouse pointer over the IComponent
		 *
		 * @param	event the MouseMoveEvent that holds the location of the
		 *			mouse */
		virtual void onHover(const MouseMoveEvent& event) = 0;

		/** Handles a mouse click on the IComponent
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onClick(const MouseButtonEvent& event) = 0;

		/** Handles a mouse click release on the IComponent
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onRelease(const MouseButtonEvent& event) = 0;
	};

}

#endif		// I_COMPONENT_H
