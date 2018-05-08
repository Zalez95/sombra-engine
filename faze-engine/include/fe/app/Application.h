#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <vector>
#include <string>

namespace fe { namespace window { class WindowSystem; }}
namespace fe { namespace graphics { class GraphicsSystem; }}
namespace fe { namespace physics { class PhysicsEngine; }}
namespace fe { namespace audio { class AudioEngine; }}

namespace fe { namespace app {

	struct Entity;
	class InputManager;
	class GraphicsManager;
	class PhysicsManager;
	class AudioManager;


	/**
	 * Class Application, TODO: complete documentation
	 */
	class Application
	{
	protected:	// Nested Types
		typedef std::unique_ptr<Entity> EntityUPtr;

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
		audio::AudioEngine* mAudioEngine;

		/** The Entities that exists currently in the application */
		std::vector<EntityUPtr> mEntities;

		/** The managers that hold the data of the entities */
		InputManager* mInputManager;
		GraphicsManager* mGraphicsManager;
		PhysicsManager* mPhysicsManager;
		AudioManager* mAudioManager;

	public:		// Functions
		/** Creates a new Game */
		Application(
			const std::string& title, int width, int height,
			float updateTime
		);

		/** Class destructor */
		~Application();

		/** Runs the Game
		 *
		 * @return	true if the Game exited succesfully, false otherwise */
		bool run();
	protected:
		/**  Function used to initialize the application's entities data */
		virtual void init() = 0;

		/** Function used to remove the application's entities data */
		virtual void end() = 0;
	};

}}

#endif		// APPLICATION_H
