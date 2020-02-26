#include <se/utils/Log.h>
#include <se/window/WindowSystem.h>
#include "SettingsMenuController.h"
#include "SettingsMenuView.h"
#include "Game.h"

namespace game {

	SettingsMenuController::SettingsMenuController(GameData& gameData) : IGameScreen(gameData), mView(nullptr)
	{
		SOMBRA_DEBUG_LOG << "start";
		mView = new SettingsMenuView(*mGameData.layer2D, *mGameData.guiManager, *this);
		SOMBRA_DEBUG_LOG << "end";
	}


	SettingsMenuController::~SettingsMenuController()
	{
		SOMBRA_DEBUG_LOG << "start";
		delete mView;
		SOMBRA_DEBUG_LOG << "end";
	}


	void SettingsMenuController::update(float /*deltaTime*/) {}


	void SettingsMenuController::onBack()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGameData.stateMachine->submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu));
		SOMBRA_DEBUG_LOG << "end";
	}


	void SettingsMenuController::onWindow(ButtonOption option)
	{
		if (option == ButtonOption::Left) {
			mView->setWindowed(SettingsMenuView::SelectionLabel::Windowed);
			mGameData.windowSystem->setFullscreen(false);
		}
		else {
			mView->setWindowed(SettingsMenuView::SelectionLabel::FullScreen);
			mGameData.windowSystem->setFullscreen(true);
		}
	}


	void SettingsMenuController::onVSync(ButtonOption option)
	{
		if (option == ButtonOption::Left) {
			mView->setVSync(SettingsMenuView::SelectionLabel::No);
			mGameData.windowSystem->setVSync(false);
		}
		else {
			mView->setVSync(SettingsMenuView::SelectionLabel::Yes);
			mGameData.windowSystem->setVSync(true);
		}
	}

}
