#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <vector>
#include <string>

namespace se::window { class WindowSystem; }
namespace se::graphics { class GraphicsSystem; }
namespace se::physics { class PhysicsEngine; }
namespace se::collision { class CollisionWorld; }
namespace se::animation { class AnimationSystem; }
namespace se::audio { class AudioEngine; }

namespace se::app {

	struct Entity;
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
		using EntityUPtr = std::unique_ptr<Entity>;

		/** The different states in which the application could be */
		enum class AppState
		{
			Error,
			Running,
			Stopped
		};

	protected:	// Attributes
		static constexpr float kBaseBias			= 0.5f;
		static constexpr float kMinFDifference		= 0.00001f;
		static constexpr float kContactPrecision	= 0.0000001f;
		static constexpr float kContactSeparation	= 0.00001f;

		const float mUpdateTime;

		/** The state of the application */
		AppState mState;

		window::WindowSystem* mWindowSystem;
		graphics::GraphicsSystem* mGraphicsSystem;
		physics::PhysicsEngine* mPhysicsEngine;
		collision::CollisionWorld* mCollisionWorld;
		animation::AnimationSystem* mAnimationSystem;
		audio::AudioEngine* mAudioEngine;

		/** The Entities that exists currently in the application */
		std::vector<EntityUPtr> mEntities;

		/** The managers that hold the data of the entities */
		EventManager* mEventManager;
		InputManager* mInputManager;
		GraphicsManager* mGraphicsManager;
		PhysicsManager* mPhysicsManager;
		CollisionManager* mCollisionManager;
		AnimationManager* mAnimationManager;
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
