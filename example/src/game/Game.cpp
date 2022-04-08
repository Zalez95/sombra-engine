#include <numeric>
#include <se/utils/Log.h>
#include <se/app/Repository.h>
#include <se/app/io/FontReader.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/physics/RigidBodyWorld.h>
#include "Game.h"
#include "Level.h"
#include "MainMenuController.h"
#include "SettingsMenuController.h"
#include "GameMenuController.h"

namespace game {

	Game::Game() :
		Application(
			se::window::WindowData{ kTitle, static_cast<int>(kWidths[0]), static_cast<int>(kHeights[0]) },
			se::physics::WorldProperties{
				kMotionBias, kPhysicsWorldAABB, kMaxCollidingRBs, kCoarseCollisionEpsilon, kMaxCollisionIterations,
				kMinFDifference, kContactPrecision, kContactSeparation, kRaycastPrecision, kCollisionBeta,
				kCollisionRestitutionFactor, kCollisionSlopPenetration, kCollisionSlopRestitution,
				kFrictionGravityAcceleration, kMaxConstraintIterations, kNumPhysicsThreads
			},
			kAudioDeviceId,
			kUpdateTime
		),
		mStateMachine(nullptr), mFPSText(nullptr)
	{
		if (mState == AppState::Error) {
			return;
		}

		// State Machine
		auto fClearAllScreens = [this]() {
			for (auto& screen : mCurrentGameScreens) {
				delete screen;
			}
			mCurrentGameScreens.clear();
		};

		auto fLoadMainMenu = [this, fClearAllScreens]() {
			fClearAllScreens();
			mCurrentGameScreens.push_back( new MainMenuController(*this) );
		};
		auto fLoadLevel = [this, fClearAllScreens]() {
			fClearAllScreens();
			mCurrentGameScreens.push_back( new Level(*this) );
		};
		auto fLoadSettings = [this, fClearAllScreens]() {
			fClearAllScreens();
			mCurrentGameScreens.push_back( new SettingsMenuController(*this) );
		};
		auto fAddGameMenu = [this]() {
			dynamic_cast<Level*>(mCurrentGameScreens.back())->setHandleInput(false);
			mCurrentGameScreens.push_back( new GameMenuController(*this) );
		};
		auto fRemoveGameMenu = [this]() {
			delete mCurrentGameScreens.back();
			mCurrentGameScreens.pop_back();
			dynamic_cast<Level*>(mCurrentGameScreens.back())->setHandleInput(true);
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

		mStateMachine = new se::utils::StateMachine(
			mGameTransitions.data(), mGameTransitions.size(),
			static_cast<se::utils::StateMachine::State>(GameState::Start)
		);

		try {
			// Font load
			auto arialSPtr = std::make_shared<se::graphics::Font>();
			std::vector<char> characterSet(128);
			std::iota(characterSet.begin(), characterSet.end(), '\0');
			if (!se::app::FontReader::read(mExternalTools->graphicsEngine->getContext(), "res/fonts/arial.ttf", characterSet, { 48, 48 }, { 1280, 720 }, *arialSPtr)) {
				throw std::runtime_error("Error reading the font file");
			}
			auto arial = mRepository->insert(std::move(arialSPtr), "arial");
			arial.setFakeUser();

			mFPSText = new se::graphics::RenderableText(glm::vec2(0.0f), glm::vec2(16.0f), arial.get(), { 0.0f, 1.0f, 0.0f, 1.0f });
			mFPSText->addTechnique(mRepository->findByName<se::graphics::Technique>("technique2D").get());
			mFPSText->setZIndex(255);
			mExternalTools->graphicsEngine->addRenderable(mFPSText);
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << "Error: " << e.what();
			mState = AppState::Error;
		}

		// Check if the game was loaded succesfully
		if (mState != AppState::Error) {
			// Load main menu
			mStateMachine->submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu));
		}

		mAccumulatedTime = 0.0f;
		mNumFrames = 0;
	}


	Game::~Game()
	{
		if (mStateMachine) {
			delete mStateMachine;
		}

		for (auto& screen : mCurrentGameScreens) {
			delete screen;
		}

		if (mFPSText) {
			mExternalTools->graphicsEngine->removeRenderable(mFPSText);
			delete mFPSText;
		}
	}

// Private functions
	void Game::onUpdate(float deltaTime, float timeSinceStart)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		mAccumulatedTime += deltaTime;
		mNumFrames++;
		if (mAccumulatedTime > 1.0f) {
			mFPSText->setText(std::to_string(mNumFrames));
			mAccumulatedTime = 0.0f;
			mNumFrames = 0;
		}

		mStateMachine->handleEvents();
		Application::onUpdate(deltaTime, timeSinceStart);
	}

}
