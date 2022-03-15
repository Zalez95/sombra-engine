#include <algorithm>
#include "se/utils/Log.h"
#include "se/utils/ThreadPool.h"
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
		mThreadPool(nullptr), mExternalTools(nullptr), mEventManager(nullptr),
		mRepository(nullptr), mEntityDatabase(nullptr),
		mAppRenderer(nullptr), mGUIManager(nullptr)
	{
		SOMBRA_INFO_LOG << "Creating the Application";

		try {
			// We need at least 1 extra thread for loading
			std::size_t numThreads = std::min(std::thread::hardware_concurrency(), 1u);
			mThreadPool = new utils::ThreadPool(numThreads);

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
			mRepository->init<ProgramRef>();
			mRepository->init<TextureRef>();
			mRepository->init<MeshRef>();
			mRepository->init<graphics::Pass>();
			mRepository->init<graphics::Technique>();
			mRepository->init<graphics::Font>();
			mRepository->init<RenderableShaderStep>();
			mRepository->init<RenderableShader>();
			mRepository->init<Script>();

			// Entities
			mEntityDatabase = new EntityDatabase(kMaxEntities);
			mEntityDatabase->addComponentTable<TagComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<TransformsComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<SkinComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<AnimationComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<CameraComponent>(kMaxCameras);
			mEntityDatabase->addComponentTable<LightComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<LightProbeComponent>(kMaxLightProbes);
			mEntityDatabase->addComponentTable<MeshComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<TerrainComponent>(kMaxTerrains);
			mEntityDatabase->addComponentTable<ParticleSystemComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<RigidBodyComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<ScriptComponent>(kMaxEntities);
			mEntityDatabase->addComponentTable<AudioSourceComponent>(kMaxEntities);

			// Systems
			mSystems.push_back(new InputSystem(*this));
			mSystems.push_back(new ScriptSystem(*this));
			mSystems.push_back(new AnimationSystem(*this));
			mSystems.push_back(new PhysicsSystem(*this));
			mSystems.push_back(new AudioSystem(*this));
			mAppRenderer = new AppRenderer(*this, windowConfig.width, windowConfig.height);
			mSystems.push_back(mAppRenderer);
			mSystems.push_back(new CameraSystem(*this));
			mSystems.push_back(new LightSystem(*this, kShadowSplitLogFactor));
			mSystems.push_back(new LightProbeSystem(*this));
			mSystems.push_back(new TerrainSystem(*this));
			mSystems.push_back(new MeshSystem(*this));
			mSystems.push_back(new ParticleSystemSystem(*this));

			// GUI
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

		if (mEntityDatabase) {
			mEntityDatabase->executeQuery([](EntityDatabase::Query& query) { query.clearEntities(); });
		}

		if (mGUIManager) { delete mGUIManager; }
		for (auto itSys = mSystems.rbegin(); itSys != mSystems.rend(); ++itSys) {
			delete *itSys;
		}
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
		if (mThreadPool) { delete mThreadPool; }

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


	void Application::onUpdate(float deltaTime)
	{
		utils::TimeGuard t0("onUpdate");
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		mExternalTools->windowManager->update();
		for (ISystem* system : mSystems) {
			system->setDeltaTime(deltaTime);
			system->update();
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
