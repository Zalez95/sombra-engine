#ifndef APPLICATION_H
#define APPLICATION_H

#include <glm/glm.hpp>
#include "events/EventManager.h"
#include "../window/WindowManager.h"
#include "../collision/CollisionWorld.h"
#include "ECS.h"

namespace se::graphics { struct GraphicsData; class GraphicsEngine; }
namespace se::physics { class PhysicsEngine; }
namespace se::animation { class AnimationEngine; }
namespace se::audio { class AudioEngine; }
namespace se::utils { class TaskManager; }

namespace se::app {

	class InputSystem;
	class ScriptSystem;
	class CameraSystem;
	class ShadowSystem;
	class AppRenderer;
	class MeshSystem;
	class TerrainSystem;
	class ParticleSystemSystem;
	class DynamicsSystem;
	class ConstraintsSystem;
	class CollisionSystem;
	class AnimationSystem;
	class AudioSystem;
	class GUIManager;
	class Repository;


	/**
	 * Class Application, it's the class that every App must inherit from
	 * to get access to all the SOMBRA managers and systems and to be updated
	 * at a constant rate.
	 */
	class Application : public IEventListener
	{
	protected:	// Nested Types
		/** The different states in which the Application could be */
		enum class AppState
		{
			Error,
			Running,
			Stopped
		};
	public:
		/** Struct ExternalTools, holds the External tools used for running the
		 * Application */
		struct ExternalTools
		{
			window::WindowManager* windowManager;
			graphics::GraphicsEngine* graphicsEngine;
			physics::PhysicsEngine* physicsEngine;
			collision::CollisionWorld* collisionWorld;
			animation::AnimationEngine* animationEngine;
			audio::AudioEngine* audioEngine;
		};

	protected:	// Attributes
		static constexpr int kMaxEntities			= 1024;
		static constexpr int kMaxTerrains			= 4;
		static constexpr int kMaxCameras			= 4;
		static constexpr int kMaxLightProbes		= 1;
		static constexpr int kMaxTasks				= 1024;
		static constexpr int kShadowResolution		= 1024;
		static constexpr float kBaseBias			= 0.1f;
		const glm::vec3 kMinPhysicsAABB = glm::vec3(-1000.0f);
		const glm::vec3 kMaxPhysicsAABB = glm::vec3( 1000.0f);

		/** The minimum elapsed time between updates in seconds */
		const float mUpdateTime;

		/** The variable used for stopping the main loop */
		bool mStopRunning;

		/** The state of the Application */
		AppState mState;

		/** The TaskManager used for used for multithreading */
		utils::TaskManager* mTaskManager;

		/** The external tools/engines */
		ExternalTools* mExternalTools;

		/** The EventManager used for the communication between the Application
		 * Systems */
		EventManager* mEventManager;

		/** The Repository that holds all the shared data of the Application.
		 * All the data added to this Repository will remain until the
		 * Application ends */
		Repository* mRepository;

		/** The EntityDatabase that holds all the Application Entities and
		 * their Components */
		EntityDatabase* mEntityDatabase;

		/** The Systems that hold and update the data of the entities */
		InputSystem* mInputSystem;
		ScriptSystem* mScriptSystem;
		CameraSystem* mCameraSystem;
		ShadowSystem* mShadowSystem;
		AppRenderer* mAppRenderer;
		MeshSystem* mMeshSystem;
		TerrainSystem* mTerrainSystem;
		ParticleSystemSystem* mParticleSystemSystem;
		DynamicsSystem* mDynamicsSystem;
		ConstraintsSystem* mConstraintsSystem;
		CollisionSystem* mCollisionSystem;
		AnimationSystem* mAnimationSystem;
		AudioSystem* mAudioSystem;

		/** The GUIManager used for drawing and updating the GUI */
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

		/** @return	a reference to the ExternalTools of the Application */
		ExternalTools& getExternalTools() { return *mExternalTools; };

		/** @return	a reference to the EventManager of the Application */
		EventManager& getEventManager() { return *mEventManager; };

		/** @return	a reference to the EntityDatabase of the Application */
		EntityDatabase& getEntityDatabase() { return *mEntityDatabase; };

		/** @return	a reference to the Repository of the Application */
		Repository& getRepository() { return *mRepository; };

		/** @return	a reference to the GUIManager of the Application */
		GUIManager& getGUIManager() { return *mGUIManager; };

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual void notify(const IEvent&) override {};

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
