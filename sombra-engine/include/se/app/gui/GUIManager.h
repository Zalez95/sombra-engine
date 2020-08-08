#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include "../../graphics/GraphicsEngine.h"
#include "../../utils/Repository.h"
#include "../events/EventManager.h"
#include "../events/ResizeEvent.h"
#include "../events/MouseEvent.h"
#include "ComposedComponent.h"

namespace se::app {

	/**
	 * Class GUIManager, it's the manager used for notifying of the user input
	 * and window updates to the IComponents of the user interface
	 */
	class GUIManager : public IEventListener
	{
	private:	// Attributes
		/** The EventManager that will notify the events */
		EventManager& mEventManager;

		/** The GraphicsEngine used for rendering the GUI IComponents */
		graphics::GraphicsEngine& mGraphicsEngine;

		/** The Repository that holds the graphics data */
		utils::Repository& mRepository;

		/** The ComposedComponent that will hold every IComponent added to the
		 * GUIManager */
		ComposedComponent mRootComponent;

	public:		// Functions
		/** Creates a new GUIManager
		 *
		 * @param	eventManager a reference to the EventManager that the
		 *			GUIManager will be subscribed to
		 * @param	graphicsEngine a reference to the GraphicsEngine used for
		 *			rendering the GUI IComponents
		 * @param	repository a reference to the Repository that holds the
		 *			graphics data
		 * @param	initialWindowSize the initial window size
		 * @note	it will create a program2D and a technique2D used for
		 *			drawing the IComponents and store them in the
		 *			GraphicsManager Repositories */
		GUIManager(
			EventManager& eventManager,
			graphics::GraphicsEngine& graphicsEngine,
			utils::Repository& repository, const glm::vec2& initialWindowSize
		);

		/** Class destructor */
		~GUIManager();

		/** @return	the GraphicsEngine of the GUIManager */
		graphics::GraphicsEngine& getGraphicsEngine()
		{ return mGraphicsEngine; };

		/** @return	the Repository of the GUIManager */
		utils::Repository& getRepository()
		{ return mRepository; };

		/** Adds the given component to the GUIComponent
		 *
		 * @param	component a pointer to the IComponent to add
		 * @param	anchor the Anchor where the given IComponent will be
		 *			added so it can change it's position when the window size
		 *			changes
		 * @param	proportions the Proportions that the given IComponent must
		 *			have so it can be resized when the window does so
		 * @note	the child IComponent will be added on top of the current
		 *			one (with the next z-index) */
		void add(
			IComponent* component,
			const Anchor& anchor, const Proportions& proportions
		);

		/** Removes the given component from the GUIComponent
		 *
		 * @param	component a pointer to the IComponent to remove */
		void remove(IComponent* component);

		/** Notifies the GUIManager of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;
	private:
		/** Handles the given ResizeEvent by updating the GUI size
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);

		/** Handles the given MouseEvent by dispatching it to the IComponents
		 * added to the GUIManager
		 *
		 * @param	event the MouseEvent to handle */
		void onMouseEvent(const MouseEvent& event);
	};

}

#endif	// GUI_MANAGER_H
