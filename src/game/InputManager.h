#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <vector>
#include "../window/WindowSystem.h"

namespace game {

	struct Entity;


	/**
	 * Class InputManager, it's a Manager used for updating the Entities's
	 * data based on the Player's input
	 */
	class InputManager
	{
	private:	// Attributes
		static const float RUN_SPEED;
		static const float JUMP_SPEED;
		static const float MOUSE_SPEED;

		/** The Window System used for checking the player's input data */
		window::WindowSystem& mWindowSystem;

		/** The Entities to update */
		std::vector<Entity*> mEntities;

	public:		// Functions
		/** Creates a new InputManager
		 * 
		 * @param	windowSystem the WindowSystem used for Checking the
		 *			input of the player */

		InputManager(window::WindowSystem& windowSystem) :
			mWindowSystem(windowSystem) { resetMousePosition(); }

		/** Class destructor */
		~InputManager() {};
		
		/** Adds the given Entity to the InputManager 
		 * 
		 * @param	entity a pointer to the Entity to add to the
		 *			InputManager */
		void addEntity(Entity* entity);

		/** Removes the given Entity from the InputManager so it won't
		 * longer be updated
		 * 
		 * @param	entity a pointer to the Entity to remove from the
		 *			InputManager */
		void removeEntity(Entity* entity);

		/** Updates the Entities with the player input */
		void update();
	private:
		/** Updates the orientation of the given Entity with the player's
		 * mouse input
		 * 
		 * @param	entity a pointer to the Entity to update
		 * @param	inputData the current player's input data */
		void doMouseInput(
			Entity* entity, const window::InputData& inputData
		) const;

		/** Updates the locations of the given Entity with the player's
		 * mouse input
		 * 
		 * @param	entity a pointer to the Entity to update
		 * @param	inputData the current player's input data */
		void doKeyboardInput(
			Entity* entity, const window::InputData& inputData
		) const;

		/** Resets the mouse position to the center of the screen */
		void resetMousePosition() const;
	};

}

#endif		// INPUT_MANAGER_H
