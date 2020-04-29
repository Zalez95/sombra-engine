#include <chrono>
#include <thread>
#include "se/utils/Log.h"
#include "se/window/WindowSystem.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/physics/PhysicsEngine.h"
#include "se/animation/AnimationSystem.h"
#include "se/audio/AudioEngine.h"
#include "se/utils/TaskSet.h"
#include "se/app/Application.h"
#include "se/app/InputManager.h"
#include "se/app/GraphicsManager.h"
#include "se/app/PhysicsManager.h"
#include "se/app/CollisionManager.h"
#include "se/app/AnimationManager.h"
#include "se/app/AudioManager.h"
#include "se/app/events/EventManager.h"
#include "se/app/gui/GUIManager.h"

namespace se::app {

	Application::Application(
		const window::WindowData& windowConfig,
		const graphics::GraphicsData& graphicsConfig,
		const collision::CollisionWorldData& collisionConfig,
		float updateTime
	) : mUpdateTime(updateTime), mState(AppState::Stopped), mStopRunning(false),
		mWindowSystem(nullptr), mGraphicsEngine(nullptr), mPhysicsEngine(nullptr), mCollisionWorld(nullptr),
		mAnimationSystem(nullptr), mAudioEngine(nullptr), mTaskManager(nullptr),
		mEventManager(nullptr), mInputManager(nullptr), mGraphicsManager(nullptr), mPhysicsManager(nullptr),
		mCollisionManager(nullptr), mAnimationManager(nullptr), mAudioManager(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the Application";
		try {
			// Tasks
			mTaskManager = new utils::TaskManager(kMaxTasks);

			// Events
			mEventManager = new EventManager();

			// Window
			mWindowSystem = new window::WindowSystem(windowConfig);

			// Input
			mInputManager = new InputManager(*mWindowSystem, *mEventManager);

			// Graphics
			mGraphicsEngine = new graphics::GraphicsEngine(graphicsConfig);
			mGraphicsManager = new GraphicsManager(*mGraphicsEngine, *mEventManager);
			mGUIManager = new GUIManager(*mEventManager, *mGraphicsManager, { windowConfig.width, windowConfig.height });

			// Physics
			mPhysicsEngine = new physics::PhysicsEngine(kBaseBias);
			mPhysicsManager = new PhysicsManager(*mPhysicsEngine, *mEventManager);

			// Collision
			mCollisionWorld = new collision::CollisionWorld(collisionConfig);
			mCollisionManager = new CollisionManager(*mCollisionWorld, *mEventManager);

			// Animation
			mAnimationSystem = new animation::AnimationSystem();
			mAnimationManager = new AnimationManager(*mAnimationSystem);

			// Audio
			mAudioEngine = new audio::AudioEngine();
			mAudioManager = new AudioManager(*mAudioEngine);
		}
		catch (std::exception& e) {
			mState = AppState::Error;
			SOMBRA_ERROR_LOG << " Error while creating the Application: " << e.what();
		}
		SOMBRA_INFO_LOG << "Application created";
	}


	Application::~Application()
	{
		SOMBRA_INFO_LOG << "Deleting the Application";
		if (mGUIManager) { delete mGUIManager; }
		if (mAudioManager) { delete mAudioManager; }
		if (mAudioEngine) { delete mAudioEngine; }
		if (mAnimationManager) { delete mAnimationManager; }
		if (mAnimationSystem) { delete mAnimationSystem; }
		if (mCollisionManager) { delete mCollisionManager; }
		if (mCollisionWorld) { delete mCollisionWorld; }
		if (mPhysicsManager) { delete mPhysicsManager; }
		if (mPhysicsEngine) { delete mPhysicsEngine; }
		if (mGraphicsManager) { delete mGraphicsManager; }
		if (mGraphicsEngine) { delete mGraphicsEngine; }
		if (mInputManager) { delete mInputManager; }
		if (mWindowSystem) { delete mWindowSystem; }
		if (mEventManager) { delete mEventManager; }
		if (mTaskManager) { delete mTaskManager; }
		SOMBRA_INFO_LOG << "Application deleted";
	}


	void Application::start()
	{
		SOMBRA_INFO_LOG << "Starting the Application";

		run();
	}


	void Application::stop()
	{
		SOMBRA_INFO_LOG << "Stopping the Application";

		if (mState == AppState::Running) {
			mStopRunning = true;
		}
	}

// Private functions
	bool Application::run()
	{
		SOMBRA_INFO_LOG << "Start running";

		if (mState != AppState::Stopped) {
			SOMBRA_ERROR_LOG << "Bad initial state";
			return false;
		}

		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		mState = AppState::Running;
		mStopRunning = false;
		auto lastTP = std::chrono::high_resolution_clock::now();
		while (!mStopRunning) {
			// Calculate the elapsed time since the last update
			auto currentTP = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> durationInSeconds = currentTP - lastTP;

			float deltaTime = durationInSeconds.count();
			/*float waitTime = mUpdateTime - deltaTime;
			if (waitTime > 0) {
				SOMBRA_DEBUG_LOG << "Wait " << waitTime << " seconds";
				std::this_thread::sleep_for( std::chrono::duration<float>(waitTime) );
			}
			else {*/
				lastTP = currentTP;

				// Retrieve the input
				onInput();

				// Update the Systems
				onUpdate(deltaTime);

				// Draw
				onRender();
			//}
		}

		mState = AppState::Stopped;

		SOMBRA_INFO_LOG << "End running";
		return true;
	}


	void Application::onInput()
	{
		SOMBRA_DEBUG_LOG << "Init";
		mWindowSystem->update();
		mInputManager->update();
		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		utils::TaskSet taskSet(*mTaskManager);
		auto animationTask = taskSet.createTask([&]() { mAnimationManager->update(deltaTime); });
		auto dynamicsTask = taskSet.createTask([&]() { mPhysicsManager->doDynamics(deltaTime); });
		auto collisionTask = taskSet.createTask([&]() { mCollisionManager->update(deltaTime); });
		auto constraintsTask = taskSet.createTask([&]() { mPhysicsManager->doConstraints(deltaTime); });
		auto audioTask = taskSet.createTask([&]() { mAudioManager->update(); });

		taskSet.depends(collisionTask, dynamicsTask);
		taskSet.depends(constraintsTask, collisionTask);
		taskSet.depends(audioTask, constraintsTask);
		taskSet.depends(audioTask, animationTask);

		taskSet.submitAndWait();

		// The GraphicsManager update function must be called from thread 0
		mGraphicsManager->update();

		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::onRender()
	{
		SOMBRA_DEBUG_LOG << "Init";
		mGraphicsManager->render();
		mWindowSystem->swapBuffers();
		SOMBRA_DEBUG_LOG << "End";
	}

}
