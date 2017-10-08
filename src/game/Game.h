#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>

namespace window { class WindowSystem; }
namespace graphics { class GraphicsSystem; }
namespace physics { class PhysicsEngine; }
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
	    static const unsigned int WIDTH		= 1280;
	    static const unsigned int HEIGHT	= 720;
	    static const float UPDATE_TIME;
		static const unsigned int NUM_CUBES;

	private:	// Nested Types
		typedef std::unique_ptr<Entity> EntityUPtr;

		/** The different states in which the game could be */
		enum GameState
		{
			ERROR,
			RUNNING,
			STOPPED
		};

	private:	// Attributes
		/** The state of the game */
		GameState mState;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;
		physics::PhysicsEngine* mPhysicsEngine;

		/** The Entities that currently are in the game */
		std::vector<EntityUPtr> mEntities;

		/** The manager that holds the data of the entities */
		InputManager* mInputManager;
		PhysicsManager* mPhysicsManager;
		GraphicsManager* mGraphicsManager;

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
