#include <se/utils/Log.h>
#include <se/window/WindowManager.h>
#include <se/app/events/ResizeEvent.h>
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


	void SettingsMenuController::onResolution(ButtonOption option)
	{
		if (option == ButtonOption::Left) {
			mGame.getEventManager().publish(new se::app::RendererResolutionEvent(kWidths[0], kHeights[0]));
		}
		else {
			mGame.getEventManager().publish(new se::app::RendererResolutionEvent(kWidths[1], kHeights[1]));
		}
	}

}
