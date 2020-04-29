#include <se/utils/Log.h>
#include "MainMenuController.h"
#include "MainMenuView.h"
#include "Game.h"

namespace game {

	MainMenuController::MainMenuController(GameData& gameData) : IGameScreen(gameData), mView(nullptr)
	{
		SOMBRA_DEBUG_LOG << "start";
		mView = new MainMenuView(*mGameData.guiManager, *this);
		SOMBRA_DEBUG_LOG << "end";
	}


	MainMenuController::~MainMenuController()
	{
		SOMBRA_DEBUG_LOG << "start";
		delete mView;
		SOMBRA_DEBUG_LOG << "end";
	}


	void MainMenuController::update(float /*deltaTime*/) {}


	void MainMenuController::onStart()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGameData.stateMachine->submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::StartLevel));
		SOMBRA_DEBUG_LOG << "end";
	}


	void MainMenuController::onConfig()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGameData.stateMachine->submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::GoToSettings));
		SOMBRA_DEBUG_LOG << "end";
	}


	void MainMenuController::onQuit()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGameData.stateMachine->submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::Quit));
		SOMBRA_DEBUG_LOG << "end";
	}

}
