#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <deque>
#include "../window/WindowSystem.h"
#include "ISystem.h"

namespace se::app {

	/**
	 * Class InputSystem, it's a System used for handling the user input at a
	 * constant time interval
	 */
	class InputSystem : public ISystem
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
		/** Creates a new InputSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	windowSystem the WindowSystem used for Checking the
		 *			input of the player
		 * @param	eventManager a reference to the EventManager where the
		 *			InputSystem will submit its events */
		InputSystem(
			EntityDatabase& entityDatabase, window::WindowSystem& windowSystem,
			EventManager& eventManager
		);

		/** Submits all the user input events */
		virtual void update() override;
	};

}

#endif		// INPUT_SYSTEM_H
