#include <numeric>
#include <se/utils/Log.h>
#include <se/utils/Repository.h>
#include <se/app/loaders/FontReader.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/window/WindowSystem.h>
#include <se/collision/CollisionWorld.h>
#include "Game.h"
#include "Level.h"
#include "MainMenuController.h"
#include "SettingsMenuController.h"
#include "GameMenuController.h"

namespace game {

	Game::Game() :
		Application(
			{ kTitle, kWidth, kHeight },
			{ kMinFDifference, kContactPrecision, kContactSeparation, kMaxManifolds, kMaxRayCasterIterations },
			kUpdateTime
		),
		mGameData{}
	{
		mGameData.taskManager = mTaskManager;
		mGameData.eventManager = mEventManager;
		mGameData.entityDatabase = mEntityDatabase;
		mGameData.repository = mRepository;

		mGameData.windowSystem = mWindowSystem;
		mGameData.graphicsEngine = mGraphicsEngine;
		mGameData.physicsEngine = mPhysicsEngine;
		mGameData.collisionWorld = mCollisionWorld;
		mGameData.animationEngine = mAnimationEngine;
		mGameData.audioEngine = mAudioEngine;

		mGameData.inputSystem = mInputSystem;
		mGameData.cameraSystem = mCameraSystem;
		mGameData.appRenderer = mAppRenderer;
		mGameData.rMeshSystem = mRMeshSystem;
		mGameData.rTerrainSystem = mRTerrainSystem;
		mGameData.dynamicsSystem = mDynamicsSystem;
		mGameData.constraintsSystem = mConstraintsSystem;
		mGameData.collisionSystem = mCollisionSystem;
		mGameData.animationSystem = mAnimationSystem;
		mGameData.audioSystem = mAudioSystem;
		mGameData.guiManager = mGUIManager;

		// State Machine
		auto fClearAllScreens = [this]() {
			for (auto& screen : mGameData.currentGameScreens) {
				delete screen;
			}
			mGameData.currentGameScreens.clear();
		};

		auto fLoadMainMenu = [this, fClearAllScreens]() {
			fClearAllScreens();
			mGameData.currentGameScreens.push_back( new MainMenuController(mGameData) );
		};
		auto fLoadLevel = [this, fClearAllScreens]() {
			fClearAllScreens();
			mGameData.currentGameScreens.push_back( new Level(mGameData) );
		};
		auto fLoadSettings = [this, fClearAllScreens]() {
			fClearAllScreens();
			mGameData.currentGameScreens.push_back( new SettingsMenuController(mGameData) );
		};
		auto fAddGameMenu = [this]() {
			dynamic_cast<Level*>(mGameData.currentGameScreens.back())->setHandleInput(false);
			mGameData.currentGameScreens.push_back( new GameMenuController(mGameData) );
		};
		auto fRemoveGameMenu = [this]() {
			delete mGameData.currentGameScreens.back();
			mGameData.currentGameScreens.pop_back();
			dynamic_cast<Level*>(mGameData.currentGameScreens.back())->setHandleInput(true);
		};
		auto fStop = [this, fClearAllScreens]() {
			fClearAllScreens();
			stop();
		};

		mGameTransitions = {
			{
				static_cast<se::utils::StateMachine::State>(GameState::Start),
				static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu),
				static_cast<se::utils::StateMachine::State>(GameState::MainMenu),
				fLoadMainMenu
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::MainMenu),
				static_cast<se::utils::StateMachine::Event>(GameEvent::StartLevel),
				static_cast<se::utils::StateMachine::State>(GameState::Level),
				fLoadLevel
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::MainMenu),
				static_cast<se::utils::StateMachine::Event>(GameEvent::GoToSettings),
				static_cast<se::utils::StateMachine::State>(GameState::SettingsMenu),
				fLoadSettings
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::MainMenu),
				static_cast<se::utils::StateMachine::Event>(GameEvent::Quit),
				static_cast<se::utils::StateMachine::State>(GameState::Stopped),
				fStop
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::SettingsMenu),
				static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu),
				static_cast<se::utils::StateMachine::State>(GameState::MainMenu),
				fLoadMainMenu
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::Level),
				static_cast<se::utils::StateMachine::Event>(GameEvent::AddGameMenu),
				static_cast<se::utils::StateMachine::State>(GameState::GameMenu),
				fAddGameMenu
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::GameMenu),
				static_cast<se::utils::StateMachine::Event>(GameEvent::RemoveGameMenu),
				static_cast<se::utils::StateMachine::State>(GameState::Level),
				fRemoveGameMenu
			}, {
				static_cast<se::utils::StateMachine::State>(GameState::GameMenu),
				static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu),
				static_cast<se::utils::StateMachine::State>(GameState::MainMenu),
				fLoadMainMenu
			}
		};

		mGameData.stateMachine = new se::utils::StateMachine(
			mGameTransitions.data(), mGameTransitions.size(),
			static_cast<se::utils::StateMachine::State>(GameState::Start)
		);

		try {
			// Font load
			auto arial = std::make_unique<se::graphics::Font>();
			std::vector<char> characterSet(128);
			std::iota(characterSet.begin(), characterSet.end(), '\0');
			if (!se::app::FontReader::read("res/fonts/arial.ttf", characterSet, { 48, 48 }, { 1280, 720 }, *arial)) {
				throw std::runtime_error("Error reading the font file");
			}

			auto arialSPtr = mRepository->add(std::string("arial"), std::move(arial));
			if (!arialSPtr) {
				throw std::runtime_error("Arial Font couldn't be added to the Repository");
			}

			mGameData.fpsText = new se::graphics::RenderableText(glm::vec2(0.0f), glm::vec2(16.0f), arialSPtr, { 0.0f, 1.0f, 0.0f, 1.0f });
			mGameData.fpsText->addTechnique(mRepository->find<std::string, se::graphics::Technique>("technique2D"));
			mGameData.fpsText->setZIndex(255);
			mGraphicsEngine->addRenderable(mGameData.fpsText);
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << "Error: " << e.what();
			mState = AppState::Error;
		}

		// Check if the game was loaded succesfully
		if (mState != AppState::Error) {
			// Load main menu
			mGameData.stateMachine->submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu));
		}

		mAccumulatedTime = 0.0f;
		mNumFrames = 0;
	}


	Game::~Game()
	{
		if (mGameData.stateMachine) {
			delete mGameData.stateMachine;
		}

		for (auto& screen : mGameData.currentGameScreens) {
			delete screen;
		}

		if (mGameData.fpsText) {
			mGameData.graphicsEngine->removeRenderable(mGameData.fpsText);
			delete mGameData.fpsText;
		}
	}

// Private functions
	void Game::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		mAccumulatedTime += deltaTime;
		mNumFrames++;
		if (mAccumulatedTime > 1.0f) {
			mGameData.fpsText->setText(std::to_string(mNumFrames));
			mAccumulatedTime = 0.0f;
			mNumFrames = 0;
		}

		mGameData.stateMachine->handleEvents();
		for (auto& screen : mGameData.currentGameScreens) {
			screen->update(deltaTime);
		}

		Application::onUpdate(deltaTime);
	}

}
