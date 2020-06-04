#ifndef APPLICATION_H
#define APPLICATION_H

namespace se::window { struct WindowData; class WindowSystem; }
namespace se::graphics { struct GraphicsData; class GraphicsEngine; }
namespace se::physics { class PhysicsEngine; }
namespace se::collision { struct CollisionWorldData; class CollisionWorld; }
namespace se::animation { class AnimationSystem; }
namespace se::audio { class AudioEngine; }
namespace se::utils { class TaskManager; }

namespace se::app {

	class EventManager;
	class InputManager;
	class GraphicsManager;
	class PhysicsManager;
	class CollisionManager;
	class AnimationManager;
	class AudioManager;
	class GUIManager;


	/**
	 * Class Application, it's the class that every App must inherit from
	 * to get access to all the SOMBRA managers and systems and to be updated
	 * at a constant rate.
	 */
	class Application
	{
	protected:	// Nested Types
		/** The different states in which the Application could be */
		enum class AppState
		{
			Error,
			Running,
			Stopped
		};

	protected:	// Attributes
		static constexpr int kMaxTasks				= 1024;
		static constexpr float kBaseBias			= 0.1f;
		static constexpr float kMinFDifference		= 0.00001f;
		static constexpr float kContactPrecision	= 0.0000001f;
		static constexpr float kContactSeparation	= 0.00001f;

		/** The minimum elapsed time between updates in seconds */
		const float mUpdateTime;

		/** The state of the Application */
		AppState mState;

		/** The vairable used for stopping the main loop */
		bool mStopRunning;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsEngine* mGraphicsEngine;
		physics::PhysicsEngine* mPhysicsEngine;
		collision::CollisionWorld* mCollisionWorld;
		animation::AnimationSystem* mAnimationSystem;
		audio::AudioEngine* mAudioEngine;
		utils::TaskManager* mTaskManager;

		/** The managers that hold the data of the entities */
		EventManager* mEventManager;
		InputManager* mInputManager;
		GraphicsManager* mGraphicsManager;
		PhysicsManager* mPhysicsManager;
		CollisionManager* mCollisionManager;
		AnimationManager* mAnimationManager;
		AudioManager* mAudioManager;
		GUIManager* mGUIManager;

	public:		// Functions
		/** Creates a new Application
		 *
		 * @param	windowConfig the initial configuration with which the
		 *			window is going to be created
		 * @param	collisionConfig the initial configuration with which the
		 *			CollisionWorld is going to be created
		 * @param	updateTime the minimum elapsed time between updates in
		 *			seconds */
		Application(
			const window::WindowData& windowConfig,
			const collision::CollisionWorldData& collisionConfig,
			float updateTime
		);

		/** Class destructor */
		virtual ~Application();

		/** Function used for starting the Application
		 * @note	the current thread will be used by the Application until
		 *			@see end is called */
		void start();

		/** Function used for stopping the Application */
		void stop();
	protected:
		/** Runs the Application
		 *
		 * @return	true if the Application exited succesfully, false
		 *			otherwise */
		bool run();

		/** Retrieves all the user input */
		virtual void onInput();

		/** Updates the Application managers and systems each main loop
		 * iteration
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void onUpdate(float deltaTime);

		/** Draws to screen */
		virtual void onRender();
	};

}

#endif		// APPLICATION_H
