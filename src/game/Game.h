#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>
#include "Entity.h"
#include "Player.h"

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
	    static const unsigned int WIDTH	    = 1280;
	    static const unsigned int HEIGHT    = 720;
	    static const float FOV;
	    static const float Z_NEAR;
	    static const float Z_FAR;

	private:	// Nested Types
		typedef std::unique_ptr<Entity> EntityUPtr;

	private:	// Attributes
		bool mEnd;
		
		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;
		physics::PhysicsEngine* mPhysicsEngine;

		/** The player entity */
		Player* mPlayer;

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
	private:
		/** Updates the input data
		 *
		 * @param	delta the elapsed time since the last update */
		void input(float delta);

		/** Updates the Entities and systems of the Game
		 * 
		 * @param	delta the elapsed time since the last update */
		void update(float delta);

		/** Renders the Entities */
		void render(
			const graphics::Camera* camera,
			const std::vector<const graphics::Renderable2D*>& renderable2Ds,
			const std::vector<const graphics::Renderable3D*>& renderable3Ds,
			const std::vector<const graphics::RenderableText*>& renderableTexts,
			const std::vector<const graphics::PointLight*>& pointLights
		);
	};

}

#endif		// GAME_H
