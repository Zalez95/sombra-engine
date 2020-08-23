#include <chrono>
#include <thread>
#include "se/utils/Log.h"
#include "se/utils/TaskSet.h"
#include "se/utils/Repository.h"
#include "se/window/WindowManager.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/2D/Font.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/physics/RigidBody.h"
#include "se/physics/PhysicsEngine.h"
#include "se/collision/Collider.h"
#include "se/collision/CollisionWorld.h"
#include "se/animation/AnimationNode.h"
#include "se/animation/AnimationEngine.h"
#include "se/audio/Source.h"
#include "se/audio/AudioEngine.h"
#include "se/app/Application.h"
#include "se/app/events/EventManager.h"
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
#include "se/app/gui/GUIManager.h"
#include "se/app/TagComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/Camera.h"
#include "se/app/graphics/Skin.h"
#include "se/app/graphics/LightSource.h"
#include "se/app/graphics/LightProbe.h"

namespace se::app {

	Application::Application(
		const window::WindowData& windowConfig,
		const collision::CollisionWorldData& collisionConfig,
		float updateTime
	) : mUpdateTime(updateTime), mStopRunning(false), mState(AppState::Stopped),
		mTaskManager(nullptr), mExternalTools(nullptr), mEventManager(nullptr),
		mRepository(nullptr), mEntityDatabase(nullptr),
		mInputSystem(nullptr), mCameraSystem(nullptr), mAppRenderer(nullptr),
		mRMeshSystem(nullptr), mRTerrainSystem(nullptr), mDynamicsSystem(nullptr),
		mConstraintsSystem(nullptr), mCollisionSystem(nullptr),
		mAnimationSystem(nullptr), mAudioSystem(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the Application";

		mTaskManager = new utils::TaskManager(kMaxTasks);
		mExternalTools = new ExternalTools();
		mEventManager = new EventManager();

		// Repository
		mRepository = new utils::Repository();
		mRepository->init<std::string, graphics::Pass>();
		mRepository->init<std::string, graphics::Technique>();
		mRepository->init<std::string, graphics::Program>();
		mRepository->init<std::string, graphics::Texture>();
		mRepository->init<std::string, graphics::Font>();

		// Entities
		mEntityDatabase = new EntityDatabase(kMaxEntities);
		mEntityDatabase->addComponentTable<TagComponent>(kMaxEntities);
		mEntityDatabase->addComponentTable<TransformsComponent>(kMaxEntities);
		mEntityDatabase->addComponentTable<Skin>(kMaxEntities);
		mEntityDatabase->addComponentTable<Camera>(kMaxCameras);
		mEntityDatabase->addComponentTable<LightSource>(kMaxEntities);
		mEntityDatabase->addComponentTable<LightProbe>(kMaxLightProbes);
		mEntityDatabase->addComponentTable<MeshComponent>(kMaxEntities);
		mEntityDatabase->addComponentTable<graphics::RenderableTerrain>(kMaxTerrains);
		mEntityDatabase->addComponentTable<physics::RigidBody>(kMaxEntities);
		mEntityDatabase->addComponentTable<collision::Collider, true>(kMaxEntities);
		mEntityDatabase->addComponentTable<animation::AnimationNode>(kMaxEntities);
		mEntityDatabase->addComponentTable<audio::Source>(kMaxEntities);

		try {
			// External tools
			mExternalTools->windowManager = new window::WindowManager(windowConfig);
			mExternalTools->graphicsEngine = new graphics::GraphicsEngine();
			mExternalTools->physicsEngine = new physics::PhysicsEngine(kBaseBias);
			mExternalTools->collisionWorld = new collision::CollisionWorld(collisionConfig);
			mExternalTools->animationEngine = new animation::AnimationEngine();
			mExternalTools->audioEngine = new audio::AudioEngine();

			se::graphics::GraphicsOperations::setViewport(0, 0, windowConfig.width, windowConfig.height);

			// Systems
			mInputSystem = new InputSystem(*this);
			mCameraSystem = new CameraSystem(*this);
			mAppRenderer = new AppRenderer(*this, windowConfig.width, windowConfig.height);
			mRMeshSystem = new RMeshSystem(*this);
			mRTerrainSystem = new RTerrainSystem(*this);
			mDynamicsSystem = new DynamicsSystem(*this);
			mConstraintsSystem = new ConstraintsSystem(*this);
			mCollisionSystem = new CollisionSystem(*this);
			mAnimationSystem = new AnimationSystem(*this);
			mAudioSystem = new AudioSystem(*this);
			mGUIManager = new GUIManager(*this, { windowConfig.width, windowConfig.height });
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
		if (mAudioSystem) { delete mAudioSystem; }
		if (mAnimationSystem) { delete mAnimationSystem; }
		if (mCollisionSystem) { delete mCollisionSystem; }
		if (mConstraintsSystem) { delete mConstraintsSystem; }
		if (mDynamicsSystem) { delete mDynamicsSystem; }
		if (mRTerrainSystem) { delete mRTerrainSystem; }
		if (mRMeshSystem) { delete mRMeshSystem; }
		if (mAppRenderer) { delete mAppRenderer; }
		if (mCameraSystem) { delete mCameraSystem; }
		if (mInputSystem) { delete mInputSystem; }
		if (mExternalTools->audioEngine) { delete mExternalTools->audioEngine; }
		if (mExternalTools->animationEngine) { delete mExternalTools->animationEngine; }
		if (mExternalTools->collisionWorld) { delete mExternalTools->collisionWorld; }
		if (mExternalTools->physicsEngine) { delete mExternalTools->physicsEngine; }
		if (mExternalTools->graphicsEngine) { delete mExternalTools->graphicsEngine; }
		if (mExternalTools->windowManager) { delete mExternalTools->windowManager; }
		if (mEntityDatabase) { delete mTaskManager; }
		if (mRepository) { delete mExternalTools; }
		if (mEventManager) { delete mEventManager; }
		if (mExternalTools) { delete mRepository; }
		if (mTaskManager) { delete mEntityDatabase; }
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
		mExternalTools->windowManager->update();
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
		mExternalTools->windowManager->swapBuffers();
		SOMBRA_DEBUG_LOG << "End";
	}

}
