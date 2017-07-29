#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>
#include "Entity.h"

namespace window { class WindowSystem; }
namespace physics { class PhysicsEngine; }
namespace graphics {
	class GraphicsSystem;
	class Renderable2D;
	class Renderable3D;
	class RenderableText;
	class PointLight;
	class Camera;
}

namespace game {

	/**
	 * Class Engine
	 */
	class Game
	{
    private:    // Constants
	    static const unsigned int WIDTH = 640;
	    static const unsigned int HEIGHT = 480;
	    static const float UPDATE_TIME;
		static const unsigned int NUM_CUBES = 50;

	private:	// Nested Types
		typedef std::unique_ptr<Entity> EntityUPtr;

	private:	// Attributes
		bool mEnd;
		
		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;
		physics::PhysicsEngine* mPhysicsEngine;

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
