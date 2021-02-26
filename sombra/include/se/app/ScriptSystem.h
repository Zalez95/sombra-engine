#ifndef SCRIPT_SYSTEM_H
#define SCRIPT_SYSTEM_H

#include "events/KeyEvent.h"
#include "events/MouseEvents.h"
#include <se/app/events/ResizeEvent.h>
#include "ISystem.h"
#include "ScriptComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class ScriptSystem, it's a System used for updating the scripts of the
	 * Entities
	 */
	class ScriptSystem : public ISystem
	{
	public:		// Attributes
		/** The Application that holds the ScriptSystem */
		Application& mApplication;

		/** Holds all the user input */
		UserInput mUserInput;

	public:		// Functions
		/** Creates a new ScriptSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ScriptSystem(Application& application);

		/** Class destructor */
		virtual ~ScriptSystem();

		/** @copydoc ISystem::onNewEntity(Entity) */
		virtual void onNewEntity(Entity entity);

		/** @copydoc ISystem::onRemoveEntity(Entity) */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the scripts of the Entities */
		virtual void update() override;

		/** Notifies the ImGuiInput of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;
	private:
		/** Handles the given event
		 *
		 * @param	event the KeyEvent to handle */
		void onKeyEvent(const KeyEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseMoveEvent to handle */
		void onMouseMoveEvent(const MouseMoveEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseScrollEvent to handle */
		void onMouseScrollEvent(const MouseScrollEvent& event);

		/** Handles the given event
		 *
		 * @param	event the MouseButtonEvent to handle */
		void onMouseButtonEvent(const MouseButtonEvent& event);

		/** Handles the given event
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);
	};

}

#endif		// SCRIPT_SYSTEM_H
