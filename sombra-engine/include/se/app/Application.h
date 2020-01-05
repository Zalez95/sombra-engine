#ifndef APPLICATION_H
#define APPLICATION_H

namespace se::window { struct WindowData; class WindowSystem; }
namespace se::graphics { class GraphicsSystem; }
namespace se::physics { class PhysicsEngine; }
namespace se::collision { class CollisionWorld; }
namespace se::animation { class AnimationSystem; }
namespace se::audio { class AudioEngine; }

namespace se::app {

	class EventManager;
	class InputManager;
	class GraphicsManager;
	class PhysicsManager;
	class CollisionManager;
	class AnimationManager;
	class AudioManager;


	/**
	 * Class Application, TODO: complete documentation
	 */
	class Application
	{
	protected:	// Nested Types
		/** The different states in which the application could be */
		enum class AppState
		{
			Error,
			Running,
			Stopped
		};

	protected:	// Attributes
		static constexpr float kBaseBias			= 0.1f;
		static constexpr float kMinFDifference		= 0.00001f;
		static constexpr float kContactPrecision	= 0.0000001f;
		static constexpr float kContactSeparation	= 0.00001f;

		/** The minimum elapsed time between updates in seconds */
		const float mUpdateTime;

		/** The state of the Application */
		AppState mState;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;
		physics::PhysicsEngine* mPhysicsEngine;
		collision::CollisionWorld* mCollisionWorld;
		animation::AnimationSystem* mAnimationSystem;
		audio::AudioEngine* mAudioEngine;

		/** The managers that hold the data of the entities */
		EventManager* mEventManager;
		InputManager* mInputManager;
		GraphicsManager* mGraphicsManager;
		PhysicsManager* mPhysicsManager;
		CollisionManager* mCollisionManager;
		AnimationManager* mAnimationManager;
		AudioManager* mAudioManager;

	public:		// Functions
		/** Creates a new Application
		 *
		 * @param	title the configuration of the Application window
		 * @param	updateTime the minimum elapsed time between updates in
		 *			seconds */
		Application(const window::WindowData& windowConfig, float updateTime);

		/** Class destructor */
		virtual ~Application();

		/** Function used start the application
		 * @note	the current thread will be used by the application until
		 *			@see end is called */
		virtual void start();

		/** Function used to stop the application */
		virtual void stop();
	protected:
		/** Runs the Game
		 *
		 * @return	true if the Game exited succesfully, false otherwise */
		bool run();

		/** Retrieves all the user input */
		virtual void onInput();

		/** Updates the application managers and systems each main loop
		 * iteration
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void onUpdate(float deltaTime);

		/** Renders the scene */
		virtual void onRender();
	};

}

#endif		// APPLICATION_H
