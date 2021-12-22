#include <chrono>
#include <thread>
#include "se/utils/Log.h"
#include "se/utils/TaskSet.h"
#include "se/app/Repository.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/2D/Font.h"
#include "se/graphics/3D/Mesh.h"
#include "se/animation/AnimationEngine.h"
#include "se/audio/AudioEngine.h"
#include "se/app/Application.h"
#include "se/app/InputSystem.h"
#include "se/app/ScriptSystem.h"
#include "se/app/AppRenderer.h"
#include "se/app/LightSystem.h"
#include "se/app/LightProbeSystem.h"
#include "se/app/MeshSystem.h"
#include "se/app/TerrainSystem.h"
#include "se/app/ParticleSystemSystem.h"
#include "se/app/CameraSystem.h"
#include "se/app/PhysicsSystem.h"
#include "se/app/AnimationSystem.h"
#include "se/app/AudioSystem.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/TagComponent.h"
#include "se/app/ScriptComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/AnimationComponent.h"
#include "se/app/SkinComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/RigidBodyComponent.h"
#include "se/app/AudioSourceComponent.h"
#include "se/utils/Profiler.h"

namespace se::app {

	Application::Application(
		const window::WindowData& windowConfig,
		const physics::WorldProperties& physicsWorldProperties,
		float updateTime
	) : mUpdateTime(updateTime), mStopRunning(false), mState(AppState::Stopped),
		mTaskManager(nullptr), mExternalTools(nullptr), mEventManager(nullptr),
		mRepository(nullptr), mEntityDatabase(nullptr),
		mInputSystem(nullptr), mScriptSystem(nullptr),
		mAppRenderer(nullptr), mLightSystem(nullptr), mLightProbeSystem(nullptr),
		mMeshSystem(nullptr), mTerrainSystem(nullptr), mParticleSystemSystem(nullptr),
		mCameraSystem(nullptr), mPhysicsSystem(nullptr), mAnimationSystem(nullptr),
		mAudioSystem(nullptr), mGUIManager(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the Application";

		try {
			mTaskManager = new utils::TaskManager(kMaxTasks);

			// External tools
			mExternalTools = new ExternalTools();
			mExternalTools->windowManager = new window::WindowManager(windowConfig);
			mExternalTools->graphicsEngine = new graphics::GraphicsEngine();
			mExternalTools->rigidBodyWorld = new physics::RigidBodyWorld(physicsWorldProperties);
			mExternalTools->animationEngine = new animation::AnimationEngine();
			mExternalTools->audioEngine = new audio::AudioEngine();

			mEventManager = new EventManager();

			// Repository
			mRepository = new Repository();
			mRepository->init<graphics::Program>();
			mRepository->init<graphics::Pass>();
			mRepository->init<graphics::Technique>();
			mRepository->init<graphics::Texture>();
			mRepository->init<graphics::Font>();
			mRepository->init<graphics::Mesh>();
			mRepository->init<RenderableShaderStep>();
			mRepository->init<RenderableShader>();
			mRepository->init<Script>();

			// Entities
			mEntityDatabase = new EntityDatabase(kMaxEntities);
			mEntityDatabase->addComponentTable<TagComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<TransformsComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<SkinComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<AnimationComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<CameraComponent, false>(kMaxCameras);
			mEntityDatabase->addComponentTable<LightComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<LightProbeComponent, false>(kMaxLightProbes);
			mEntityDatabase->addComponentTable<MeshComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<TerrainComponent, false>(kMaxTerrains);
			mEntityDatabase->addComponentTable<ParticleSystemComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<RigidBodyComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<ScriptComponent, false>(kMaxEntities);
			mEntityDatabase->addComponentTable<AudioSourceComponent, false>(kMaxEntities);

			// Systems
			mInputSystem = new InputSystem(*this);
			mScriptSystem = new ScriptSystem(*this);
			mAppRenderer = new AppRenderer(*this, windowConfig.width, windowConfig.height);
			mLightSystem = new LightSystem(*this, kShadowSplitLogFactor);
			mLightProbeSystem = new LightProbeSystem(*this);
			mMeshSystem = new MeshSystem(*this);
			mTerrainSystem = new TerrainSystem(*this);
			mParticleSystemSystem = new ParticleSystemSystem(*this);
			mCameraSystem = new CameraSystem(*this);
			mPhysicsSystem = new PhysicsSystem(*this);
			mAnimationSystem = new AnimationSystem(*this);
			mAudioSystem = new AudioSystem(*this);
			mGUIManager = new GUIManager(*this, { windowConfig.width, windowConfig.height });

			SOMBRA_INFO_LOG << "Application created successfully";
		}
		catch (std::exception& e) {
			mState = AppState::Error;
			SOMBRA_FATAL_LOG << "Error while creating the Application: " << e.what();
		}
	}


	Application::~Application()
	{
		SOMBRA_INFO_LOG << "Deleting the Application";
		if (mEntityDatabase) { mEntityDatabase->clearEntities(); }
		if (mGUIManager) { delete mGUIManager; }
		if (mAudioSystem) { delete mAudioSystem; }
		if (mAnimationSystem) { delete mAnimationSystem; }
		if (mPhysicsSystem) { delete mPhysicsSystem; }
		if (mCameraSystem) { delete mCameraSystem; }
		if (mParticleSystemSystem) { delete mParticleSystemSystem; }
		if (mTerrainSystem) { delete mTerrainSystem; }
		if (mMeshSystem) { delete mMeshSystem; }
		if (mLightProbeSystem) { delete mLightProbeSystem; }
		if (mLightSystem) { delete mLightSystem; }
		if (mAppRenderer) { delete mAppRenderer; }
		if (mScriptSystem) { delete mScriptSystem; }
		if (mInputSystem) { delete mInputSystem; }
		if (mEntityDatabase) { delete mEntityDatabase; }
		if (mRepository) { delete mRepository; }
		if (mEventManager) { delete mEventManager; }
		if (mExternalTools) {
			if (mExternalTools->audioEngine) { delete mExternalTools->audioEngine; }
			if (mExternalTools->animationEngine) { delete mExternalTools->animationEngine; }
			if (mExternalTools->rigidBodyWorld) { delete mExternalTools->rigidBodyWorld; }
			if (mExternalTools->graphicsEngine) { delete mExternalTools->graphicsEngine; }
			if (mExternalTools->windowManager) { delete mExternalTools->windowManager; }
			delete mExternalTools;
		}
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
			utils::TimeGuard tf("frame");

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
		utils::TimeGuard t0("onInput");
		SOMBRA_DEBUG_LOG << "Init";
		mExternalTools->windowManager->update();
		mInputSystem->update();
		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::onUpdate(float deltaTime)
	{
		utils::TimeGuard t0("onUpdate");
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		//utils::TaskSet taskSet(*mTaskManager);
		/*auto scriptTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("scriptsys");
			mScriptSystem->setDeltaTime(deltaTime);
			mScriptSystem->update();
		}//);
		/*auto animationTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("animsys");
			mAnimationSystem->setDeltaTime(deltaTime);
			mAnimationSystem->update();
		}//);
		/*auto physicsTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("physicsssys");
			mPhysicsSystem->setDeltaTime(deltaTime);
			mPhysicsSystem->update();
		}//);
		/*auto audioTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("audiosys");
			mAudioSystem->setDeltaTime(deltaTime);
			mAudioSystem->update();
		}//);
		/*auto cameraTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("camerasys");
			mCameraSystem->setDeltaTime(deltaTime);
			mCameraSystem->update();
		}//);
		/*auto lightTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("lightsys");
			mLightSystem->setDeltaTime(deltaTime);
			mLightSystem->update();
		}//);
		/*auto lightProbeTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("lightProbesys");
			mLightProbeSystem->setDeltaTime(deltaTime);
			mLightProbeSystem->update();
		}//);
		/*auto rmeshTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("meshsys");
			mMeshSystem->setDeltaTime(deltaTime);
			mMeshSystem->update();
		}//);
		/*auto rterrainTask = taskSet.createTask([&]() */{
			utils::TimeGuard t("terrainsys");
			mTerrainSystem->setDeltaTime(deltaTime);
			mTerrainSystem->update();
		}//);

		/*taskSet.depends(animationTask, scriptTask);
		taskSet.depends(physicsTask, animationTask);
		taskSet.depends(audioTask, physicsTask);
		taskSet.depends(cameraTask, physicsTask);
		taskSet.depends(lightTask, physicsTask);
		taskSet.depends(lightProbeTask, physicsTask);
		taskSet.depends(rmeshTask, physicsTask);
		taskSet.depends(rterrainTask, cameraTask);

		taskSet.submitAndWait();*/

		// The ParticleSystemSystem update function must be called from thread 0
		{ utils::TimeGuard t("particlessys");
		mParticleSystemSystem->setDeltaTime(deltaTime);
		mParticleSystemSystem->update();
		}

		// The AppRenderer update function must be called from thread 0
		{ utils::TimeGuard t("mAppRenderer");
		mAppRenderer->setDeltaTime(deltaTime);
		mAppRenderer->update();
		}

		SOMBRA_DEBUG_LOG << "End";
	}


	void Application::onRender()
	{
		utils::TimeGuard t0("onRender");
		SOMBRA_DEBUG_LOG << "Init";
		mAppRenderer->render();
		mExternalTools->windowManager->swapBuffers();
		SOMBRA_DEBUG_LOG << "End";
	}

}
