#include <chrono>
#include <thread>
#include "se/utils/Log.h"
#include "se/window/WindowSystem.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/physics/PhysicsEngine.h"
#include "se/animation/AnimationEngine.h"
#include "se/audio/AudioEngine.h"
#include "se/utils/TaskSet.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/InputSystem.h"
#include "se/app/CameraSystem.h"
#include "se/app/AppRenderer.h"
#include "se/app/RMeshSystem.h"
#include "se/app/RTerrainSystem.h"
#include "se/app/DynamicsSystem.h"
#include "se/app/ConstraintsSystem.h"
#include "se/app/CollisionSystem.h"
#include "se/app/AnimationSystem.h"
#include "se/app/AudioSystem.h"
#include "se/app/events/EventManager.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/TagComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Camera.h"
#include "se/app/graphics/Skin.h"
#include "se/app/graphics/LightSource.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/physics/RigidBody.h"
#include "se/collision/Collider.h"
#include "se/animation/AnimationNode.h"
#include "se/audio/Source.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/2D/Font.h"

namespace se::app {

	Application::Application(
		const window::WindowData& windowConfig,
		const collision::CollisionWorldData& collisionConfig,
		float updateTime
	) : mUpdateTime(updateTime), mState(AppState::Stopped), mStopRunning(false),
		mTaskManager(nullptr), mEventManager(nullptr), mEntityDatabase(nullptr), mRepository(nullptr),
		mWindowSystem(nullptr), mGraphicsEngine(nullptr), mPhysicsEngine(nullptr),
		mCollisionWorld(nullptr), mAnimationEngine(nullptr), mAudioEngine(nullptr),
		mInputSystem(nullptr),
		mCameraSystem(nullptr), mAppRenderer(nullptr), mRMeshSystem(nullptr), mRTerrainSystem(nullptr),
		mDynamicsSystem(nullptr), mConstraintsSystem(nullptr), mCollisionSystem(nullptr),
		mAnimationSystem(nullptr), mAudioSystem(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the Application";
		try {
			// Tasks
			mTaskManager = new utils::TaskManager(kMaxTasks);

			// Events
			mEventManager = new EventManager();

			// Entities
			mEntityDatabase = new EntityDatabase(kMaxEntities);
			mEntityDatabase->addComponentTable<TagComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<TransformsComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<Skin>(kMaxEntities);
			mEntityDatabase->addComponentTable<Camera>(kMaxCameras);
			mEntityDatabase->addComponentTable<LightSource>(kMaxEntities);
			mEntityDatabase->addComponentTable<MeshComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<graphics::RenderableTerrain>(kMaxTerrains);
			mEntityDatabase->addComponentTable<physics::RigidBody>(kMaxEntities);
			mEntityDatabase->addComponentTable<collision::Collider, true>(kMaxEntities);
			mEntityDatabase->addComponentTable<animation::AnimationNode>(kMaxEntities);
			mEntityDatabase->addComponentTable<audio::Source>(kMaxEntities);

			// Repository
			mRepository = new utils::Repository();
			mRepository->init<std::string, graphics::Pass>();
			mRepository->init<std::string, graphics::Technique>();
			mRepository->init<std::string, graphics::Program>();
			mRepository->init<std::string, graphics::Texture>();
			mRepository->init<std::string, graphics::Font>();

			// Window
			mWindowSystem = new window::WindowSystem(windowConfig);

			// Input
			mInputSystem = new InputSystem(*mEntityDatabase, *mWindowSystem, *mEventManager);

			// Graphics
			mGraphicsEngine = new graphics::GraphicsEngine();
			mCameraSystem = new CameraSystem(*mEntityDatabase, windowConfig.width, windowConfig.height);
			mAppRenderer = new AppRenderer(*this, *mGraphicsEngine, *mCameraSystem, windowConfig.width, windowConfig.height);
			mRMeshSystem = new RMeshSystem(*mEntityDatabase, *mGraphicsEngine, *mCameraSystem);
			mRTerrainSystem = new RTerrainSystem(*mEntityDatabase, *mGraphicsEngine, *mCameraSystem);
			mGUIManager = new GUIManager(*mEventManager, *mGraphicsEngine, *mRepository, { windowConfig.width, windowConfig.height });
			se::graphics::GraphicsOperations::setViewport(0, 0, windowConfig.width, windowConfig.height);

			// Physics
			mPhysicsEngine = new physics::PhysicsEngine(kBaseBias);
			mDynamicsSystem = new DynamicsSystem(*mEntityDatabase, *mPhysicsEngine);
			mConstraintsSystem = new ConstraintsSystem(*mEntityDatabase, *mEventManager, *mPhysicsEngine);

			// Collision
			mCollisionWorld = new collision::CollisionWorld(collisionConfig);
			mCollisionSystem = new CollisionSystem(*mEntityDatabase, *mEventManager, *mCollisionWorld);

			// Animation
			mAnimationEngine = new animation::AnimationEngine();
			mAnimationSystem = new AnimationSystem(*mEntityDatabase, *mAnimationEngine);

			// Audio
			mAudioEngine = new audio::AudioEngine();
			mAudioSystem = new AudioSystem(*mEntityDatabase, *mAudioEngine);
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
		if (mAudioSystem) { delete mAudioSystem; }
		if (mAudioEngine) { delete mAudioEngine; }
		if (mAnimationSystem) { delete mAnimationSystem; }
		if (mAnimationEngine) { delete mAnimationEngine; }
		if (mCollisionSystem) { delete mCollisionSystem; }
		if (mCollisionWorld) { delete mCollisionWorld; }
		if (mConstraintsSystem) { delete mConstraintsSystem; }
		if (mDynamicsSystem) { delete mDynamicsSystem; }
		if (mPhysicsEngine) { delete mPhysicsEngine; }
		if (mGUIManager) { delete mGUIManager; }
		if (mRTerrainSystem) { delete mRTerrainSystem; }
		if (mRMeshSystem) { delete mRMeshSystem; }
		if (mAppRenderer) { delete mAppRenderer; }
		if (mCameraSystem) { delete mCameraSystem; }
		if (mGraphicsEngine) { delete mGraphicsEngine; }
		if (mInputSystem) { delete mInputSystem; }
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
		mInputSystem->update();
		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		utils::TaskSet taskSet(*mTaskManager);
		auto animationTask = taskSet.createTask([&]() {
			mAnimationSystem->setDeltaTime(deltaTime);
			mAnimationSystem->update();
		});
		auto dynamicsTask = taskSet.createTask([&]() {
			mDynamicsSystem->setDeltaTime(deltaTime);
			mDynamicsSystem->update();
		});
		auto collisionTask = taskSet.createTask([&]() { mCollisionSystem->update(); });
		auto constraintsTask = taskSet.createTask([&]() {
			mConstraintsSystem->setDeltaTime(deltaTime);
			mConstraintsSystem->update();
		});
		auto audioTask = taskSet.createTask([&]() { mAudioSystem->update(); });
		auto cameraTask = taskSet.createTask([&]() { mCameraSystem->update(); });
		auto rmeshTask = taskSet.createTask([&]() { mRMeshSystem->update(); });
		auto rterrainTask = taskSet.createTask([&]() { mRTerrainSystem->update(); });

		taskSet.depends(collisionTask, dynamicsTask);
		taskSet.depends(constraintsTask, collisionTask);
		taskSet.depends(audioTask, constraintsTask);
		taskSet.depends(audioTask, animationTask);
		taskSet.depends(cameraTask, constraintsTask);
		taskSet.depends(rmeshTask, constraintsTask);
		taskSet.depends(rterrainTask, cameraTask);

		taskSet.submitAndWait();

		// The GraphicsSystem update function must be called from thread 0
		mAppRenderer->update();

		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::onRender()
	{
		SOMBRA_DEBUG_LOG << "Init";
		mAppRenderer->render();
		mWindowSystem->swapBuffers();
		SOMBRA_DEBUG_LOG << "End";
	}

}
