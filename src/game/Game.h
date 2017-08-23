#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>

namespace window { class WindowSystem; }
namespace game {

	struct Entity;
	class InputManager;
	class PhysicsManager;
	class GraphicsManager;


	/**
	 * Class Engine
	 */
	class Game
	{
    private:    // Constants
	    static const unsigned int WIDTH = 640;
	    static const unsigned int HEIGHT = 480;
	    static const float UPDATE_TIME;
		static const unsigned int NUM_CUBES;

	private:	// Nested Types
		typedef std::unique_ptr<Entity> EntityUPtr;

	private:	// Attributes
		bool mEnd;
		
		window::WindowSystem* mWindowSystem;
		InputManager* mInputManager;
		PhysicsManager* mPhysicsManager;
		GraphicsManager* mGraphicsManager;

		/** The Entities that currently are in the game */
		std::vector<EntityUPtr> mEntities;

	public:		// Functions
		/** Creates a new Game */
		Game();

		/** Class destructor */
		~Game();

		/** Runs the Game
		 * 
		 * @return	true if the Game exited succesfully, false otherwise */
		bool run();
	};

}

#endif		// GAME_H
