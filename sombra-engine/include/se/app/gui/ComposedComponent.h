#ifndef COMPOSED_COMPONENT_H
#define COMPOSED_COMPONENT_H

#include <vector>
#include "IComponent.h"

namespace se::app {

	/**
	 * Class ComposedComponent, it's a GUI element that can hold multiple
	 * child IComponents inside so they can be updated when the current one
	 * is updated
	 */
	class ComposedComponent : public IComponent
	{
	protected:	// Attributes
		/** The children IComponents of the current one */
		std::vector<IComponent*> mChildren;

	public:		// Functions
		/** Class destructor */
		virtual ~ComposedComponent() = default;

		/** Handles a mouse pointer over the ComposedComponent
		 *
		 * @param	event the MouseMoveEvent that holds the location of the
		 *			mouse */
		virtual void onHover(const MouseMoveEvent& event) override;

		/** Handles a mouse click on the ComposedComponent
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onClick(const MouseButtonEvent& event) override;

		/** Handles a mouse click release on the ComposedComponent
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onRelease(const MouseButtonEvent& event) override;

		/** Adds the given IComponent as a child of the ComposedComponent
		 *
		 * @param	child a pointer to the new child IComponent */
		void add(IComponent* child);

		/** Removes the given IComponent from the ComposedComponent
		 *
		 * @param	child a pointer to the child IComponent to remove */
		void remove(IComponent* child);
	};

}

#endif		// COMPOSED_COMPONENT_H
