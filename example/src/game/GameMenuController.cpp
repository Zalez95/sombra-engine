#include <se/utils/Log.h>
#include "GameMenuController.h"
#include "GameMenuView.h"
#include "Game.h"

namespace game {

	GameMenuController::GameMenuController(Game& game) : IGameScreen(game), mView(nullptr)
	{
		SOMBRA_DEBUG_LOG << "start";
		mView = new GameMenuView(mGame, *this);
		SOMBRA_DEBUG_LOG << "end";
	}


	GameMenuController::~GameMenuController()
	{
		SOMBRA_DEBUG_LOG << "start";
		delete mView;
		SOMBRA_DEBUG_LOG << "end";
	}


	void GameMenuController::update(float /*deltaTime*/) {}


	void GameMenuController::onBack()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGame.getStateMachine().submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::RemoveGameMenu));
		SOMBRA_DEBUG_LOG << "end";
	}


	void GameMenuController::onQuit()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGame.getStateMachine().submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu));
		SOMBRA_DEBUG_LOG << "end";
	}

}