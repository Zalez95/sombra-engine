#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include "../../graphics/Technique.h"
#include "../graphics/TypeRefs.h"
#include "../events/EventManager.h"
#include "../events/ResizeEvent.h"
#include "../events/MouseEvents.h"
#include "../Repository.h"
#include "ComposedComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class GUIManager, it's the manager used for notifying of the user input
	 * and window updates to the IComponents of the user interface
	 */
	class GUIManager : public IEventListener
	{
	private:	// Attributes
		/** The Application that holds the EventManager that will notify the
		 * GUI events and the GraphicsEngine used for rendering the GUI */
		Application& mApplication;

		/** The ComposedComponent that will hold every IComponent added to the
		 * GUIManager */
		ComposedComponent mRootComponent;

		/** The projection matrix used for rendering the 2D IComponents */
		UniformVVRef<glm::mat4> mProjectionMatrix;

		/** The Program used by the GUIManager */
		Repository::ResourceRef<ProgramRef> mProgramResource;

		/** The Technique used by the GUIManager */
		Repository::ResourceRef<graphics::Technique> mTechnique;

	public:		// Functions
		/** Creates a new GUIManager
		 *
		 * @param	application a reference to the Application that holds the
		 *			GUIManager
		 * @param	initialWindowSize the initial window size
		 * @note	it will create a program2D and a technique2D used for
		 *			drawing the IComponents and store them in the
		 *			GraphicsManager Repositories */
		GUIManager(
			Application& application, const glm::vec2& initialWindowSize
		);

		/** Class destructor */
		~GUIManager();

		/** @return	a reference to the Application that holds the GUIManager */
		Application& getApplication() { return mApplication; };

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

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;
	private:
		/** Handles the given event by updating the GUI size
		 *
		 * @param	event the WindowResizeEvent to handle */
		void onWindowResizeEvent(const WindowResizeEvent& event);

		/** Handles the given event by dispatching it to the IComponents
		 * added to the GUIManager
		 *
		 * @param	event the MouseMoveEvent to handle */
		void onMouseMoveEvent(const MouseMoveEvent& event);

		/** Handles the given event by dispatching it to the IComponents
		 * added to the GUIManager
		 *
		 * @param	event the MouseButtonEvent to handle */
		void onMouseButtonEvent(const MouseButtonEvent& event);
	};

}

#endif	// GUI_MANAGER_H
