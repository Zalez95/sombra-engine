#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include "../window/WindowSystem.h"

namespace game {

	class Entity;


	/**
	 * Class InputComponent, it's a Component used for update an Entity based
	 * on the Player's input
	 */
	class InputComponent
	{
	private:	// Attributes
		static const float RUN_SPEED;
		static const float JUMP_SPEED;
		static const float MOUSE_SPEED;

		/** The Window System used for checking the player's input data */
		window::WindowSystem& mWindowSystem;

	public:		// Functions
		/** Creates a new InputComponent
		 * 
		 * @param	windowSystem the WindowSystem used for Checking the
		 *			input of the player */
		InputComponent(window::WindowSystem& windowSystem) :
	   		mWindowSystem(windowSystem) {};

		/** Class destructor */
		~InputComponent() {};

		/** Updates the given common data of the given entity with the player
		 * input
		 * 
		 * @param	entity the Entity to update
		 * @param	delta the elapsed time since the last update */
		void update(Entity& entity, float delta);
	private:
		/** Updates the orientation of the given entity with the player's
		 * mouse input
		 * 
		 * @param	entity the Entity to update
		 * @param	inputData the current player's input data
		 * @param	delta the elapsed time since the last update */
		void doMouseInput(
			Entity& entity,
			const window::InputData& inputData, float delta
		) const;

		/** Updates the locations of the given entity with the player's
		 * mouse input
		 * 
		 * @param	entity the Entity to update
		 * @param	inputData the current player's input data
		 * @param	delta the elapsed time since the last update */
		void doKeyboardInput(
			Entity& entity,
			const window::InputData& inputData, float delta
		) const;
	};

}

#endif		// INPUT_COMPONENT_H
