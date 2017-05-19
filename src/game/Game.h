#ifndef GAME_H
#define GAME_H

#include <vector>

namespace window { class WindowSystem; }
namespace graphics {
	class GraphicsSystem;
	class Renderable2D;
	class Renderable3D;
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

	private:	// Attributes
		bool mEnd;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;

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
		/** Updates the input data */
		void input();

		/** Updates the Entities and systems of the Game
		 * 
		 * @param	delta the elapsed time since the last update */
		void update(float delta);

		/** Renders the Entities */
		void render(
			const graphics::Camera* camera,
			const std::vector<const graphics::Renderable3D*>& renderable3Ds,
			const std::vector<const graphics::Renderable2D*>& renderable2Ds,
			const std::vector<const graphics::PointLight*>& pointLights
		);
	};

}

#endif		// GAME_H
