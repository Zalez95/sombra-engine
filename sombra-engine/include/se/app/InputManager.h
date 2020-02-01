#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <deque>
#include "EventManager.h"
#include "../window/WindowSystem.h"

namespace se::app {

	/**
	 * Class InputManager, it's a Manager used for handling the user input at a
	 * constant time interval
	 */
	class InputManager
	{
	private:	// Attributes
		/** A reference to the WindowSystem used for checking the player's
		 * input data */
		window::WindowSystem& mWindowSystem;

		/** The EventManager used for notifying the input events */
		EventManager& mEventManager;

		/** A queue that holds all the events that must be submitted in the
		 * next update call due to the user input from oldest to newest */
		std::deque<IEvent*> mEventQueue;

	public:		// Functions
		/** Creates a new InputManager
		 *
		 * @param	windowSystem the WindowSystem used for Checking the
		 *			input of the player
		 * @param	eventManager a reference to the EventManager where the
		 *			InputManager will submit its events */
		InputManager(
			window::WindowSystem& windowSystem,
			EventManager& eventManager
		);

		/** Submits all the user input events */
		void update();
	};

}

#endif		// INPUT_MANAGER_H
