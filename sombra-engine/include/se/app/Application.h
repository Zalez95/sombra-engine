#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <vector>
#include <string>

namespace se::window { class WindowSystem; }
namespace se::graphics { class GraphicsSystem; }
namespace se::physics { class PhysicsEngine; }
namespace se::collision { class CollisionDetector; }
namespace se::audio { class AudioEngine; }

namespace se::app {

	struct Entity;
	class InputManager;
	class GraphicsManager;
	class PhysicsManager;
	class CollisionManager;
	class AudioManager;


	/**
	 * Class Application, TODO: complete documentation
	 */
	class Application
	{
	protected:	// Nested Types
		using EntityUPtr = std::unique_ptr<Entity>;

		/** The different states in which the application could be */
		enum class AppState
		{
			ERROR,
			RUNNING,
			STOPPED
		};

	protected:	// Attributes
		/** Delay time in seconds between each iteration of the game loop */
		const float mUpdateTime;

		/** The state of the application */
		AppState mState;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;
		physics::PhysicsEngine* mPhysicsEngine;
		collision::CollisionDetector* mCollisionDetector;
		audio::AudioEngine* mAudioEngine;

		/** The Entities that exists currently in the application */
		std::vector<EntityUPtr> mEntities;

		/** The managers that hold the data of the entities */
		InputManager* mInputManager;
		GraphicsManager* mGraphicsManager;
		PhysicsManager* mPhysicsManager;
		CollisionManager* mCollisionManager;
		AudioManager* mAudioManager;

	public:		// Functions
		/** Creates a new Game */
		Application(
			const std::string& title, int width, int height,
			float updateTime
		);

		/** Class destructor */
		virtual ~Application();

		/** Runs the Game
		 *
		 * @return	true if the Game exited succesfully, false otherwise */
		virtual bool run();
	protected:
		/**  Function used to initialize the application's entities data */
		virtual void init() = 0;

		/** Function used to remove the application's entities data */
		virtual void end() = 0;
	};

}

#endif		// APPLICATION_H
