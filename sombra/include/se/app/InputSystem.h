#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <deque>
#include "events/MouseEvents.h"
#include "events/EventManager.h"
#include "ECS.h"

namespace se::app {

	class Application;


	/**
	 * Class InputSystem, it's a System used for handling the user input at a
	 * constant time interval
	 */
	class InputSystem : public ISystem, IEventListener
	{
	private:	// Attributes
		/** A reference to the Application that holds WindowManager and
		 * EventManager needed for retrieving and notifying the user input */
		Application& mApplication;

		/** A queue that holds all the events that must be submitted in the
		 * next update call due to the user input from oldest to newest */
		std::deque<IEvent*> mEventQueue;

	public:		// Functions
		/** Creates a new InputSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		InputSystem(Application& application);

		/** Class destructor */
		~InputSystem();

		/** Submits all the user input events */
		virtual void update() override;

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override
		{ return tryCall(&InputSystem::onSetMousePosEvent, event); };
	private:
		/** Handles the given event
		 *
		 * @param	event the SetMousePosEvent to handle */
		void onSetMousePosEvent(const SetMousePosEvent& event);
	};

}

#endif		// INPUT_SYSTEM_H
