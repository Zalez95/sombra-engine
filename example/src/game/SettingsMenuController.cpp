#include <se/utils/Log.h>
#include <se/window/WindowManager.h>
#include "SettingsMenuController.h"
#include "SettingsMenuView.h"
#include "Game.h"

namespace game {

	SettingsMenuController::SettingsMenuController(Game& game) : IGameScreen(game), mView(nullptr)
	{
		SOMBRA_DEBUG_LOG << "start";
		mView = new SettingsMenuView(mGame, *this);
		SOMBRA_DEBUG_LOG << "end";
	}


	SettingsMenuController::~SettingsMenuController()
	{
		SOMBRA_DEBUG_LOG << "start";
		delete mView;
		SOMBRA_DEBUG_LOG << "end";
	}


	void SettingsMenuController::onBack()
	{
		SOMBRA_DEBUG_LOG << "start";
		mGame.getStateMachine().submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::GoToMainMenu));
		SOMBRA_DEBUG_LOG << "end";
	}


	void SettingsMenuController::onWindow(ButtonOption option)
	{
		if (option == ButtonOption::Left) {
			mView->setWindowed(SettingsMenuView::SelectionLabel::Windowed);
			mGame.getExternalTools().windowManager->setFullscreen(false);
		}
		else {
			mView->setWindowed(SettingsMenuView::SelectionLabel::FullScreen);
			mGame.getExternalTools().windowManager->setFullscreen(true);
		}
	}


	void SettingsMenuController::onVSync(ButtonOption option)
	{
		if (option == ButtonOption::Left) {
			mView->setVSync(SettingsMenuView::SelectionLabel::No);
			mGame.getExternalTools().windowManager->setVSync(false);
		}
		else {
			mView->setVSync(SettingsMenuView::SelectionLabel::Yes);
			mGame.getExternalTools().windowManager->setVSync(true);
		}
	}

}
