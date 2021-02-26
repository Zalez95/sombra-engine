#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <deque>
#include "ISystem.h"
#include "events/MouseEvents.h"

namespace se::app {

	class Application;


	/**
	 * Class InputSystem, it's a System used for handling the user input at a
	 * constant time interval
	 */
	class InputSystem : public ISystem
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

		/** Notifies the ImGuiInput of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;
	private:
		/** Handles the given event
		 *
		 * @param	event the SetMousePosEvent to handle */
		void onSetMousePosEvent(const SetMousePosEvent& event);
	};

}

#endif		// INPUT_SYSTEM_H
