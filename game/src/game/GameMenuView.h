#ifndef GAME_MENU_VIEW_H
#define GAME_MENU_VIEW_H

#include <se/app/gui/Panel.h>
#include <se/app/gui/Label.h>
#include <se/app/gui/Button.h>
#include <se/app/gui/Rectangle.h>
#include <se/app/gui/GUIManager.h>
#include "GameMenuController.h"

namespace game {

	/**
	 * Class GameMenuView, it holds and configures all the IComponents used
	 * for drawing the game menu screen
	 */
	class GameMenuView
	{
	private:	// Attributes
		/** The GUIManager used for retrieving input events */
		se::app::GUIManager& mGUIManager;

		/** The controller that will handle the user input */
		GameMenuController& mController;

		/** The Panel that will Hold all the elements of the GameMenu */
		se::app::Panel mPanel;

		/** The Labels of the GameMenu */
		se::app::Label mTitleLabel;

		/** The different Buttons of the GameMenu */
		se::app::Button mBackButton, mQuitButton;

		/** The Labels of the GameMenu Buttons */
		se::app::Label mBackLabel, mQuitLabel;

	public:		// Functions
		/** Creates a new GameMenuView
		 *
		 * @param	guiManager the GUIManager to use for retrieving input
		 *			events
		 * @param	controller the GameMenuController that will handle the user
		 *			input */
		GameMenuView(
			se::app::GUIManager& guiManager,
			GameMenuController& controller
		) : mGUIManager(guiManager), mController(controller),
			mPanel(&mGUIManager), mTitleLabel(&mGUIManager),
			mBackButton(&mGUIManager, std::make_unique<se::app::Rectangle>()),
			mQuitButton(&mGUIManager, std::make_unique<se::app::Rectangle>()),
			mBackLabel(&mGUIManager), mQuitLabel(&mGUIManager)
		{
			auto arial = mGUIManager.getRepository().find<std::string, se::graphics::Font>("arial");
			if (!arial) { return; }

			mTitleLabel.setFont(arial);
			mTitleLabel.setCharacterSize({ 24, 24 });
			mTitleLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mTitleLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mTitleLabel.setText("Game Menu");
			se::app::Anchor titleButtonAnchor;
			titleButtonAnchor.relativePosition = { 0.5f, 0.1f };
			se::app::Proportions titleButtonProportions;
			titleButtonProportions.relativeSize = { 0.25f, 0.1f };
			mPanel.add(&mTitleLabel, titleButtonAnchor, titleButtonProportions);

			mBackLabel.setFont(arial);
			mBackLabel.setCharacterSize({ 24, 24 });
			mBackLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mBackLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mBackLabel.setText("Back to Game");

			mBackButton.setColor({ 1.0f, 0.5f, 0.5f, 1.0f });
			mBackButton.setLabel(&mBackLabel);
			mBackButton.setAction([this]() { mController.onBack(); });
			se::app::Anchor backButtonAnchor;
			backButtonAnchor.relativePosition = { 0.5f, 0.3f };
			se::app::Proportions backButtonProportions;
			backButtonProportions.relativeSize = { 0.25f, 0.1f };
			mPanel.add(&mBackButton, backButtonAnchor, backButtonProportions);

			mQuitLabel.setFont(arial);
			mQuitLabel.setCharacterSize({ 24, 24 });
			mQuitLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mQuitLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mQuitLabel.setText("Exit to Main Menu");

			mQuitButton.setColor({ 1.0f, 0.5f, 0.5f, 1.0f });
			mQuitButton.setLabel(&mQuitLabel);
			mQuitButton.setAction([this]() { mController.onQuit(); });
			se::app::Anchor quitButtonAnchor;
			quitButtonAnchor.relativePosition = { 0.5f, 0.45f };
			se::app::Proportions quitButtonProportions;
			quitButtonProportions.relativeSize = { 0.25f, 0.1f };
			mPanel.add(&mQuitButton, quitButtonAnchor, quitButtonProportions);

			mPanel.setColor({ 1.0f, 1.0f, 1.0f, 0.8f });
			se::app::Proportions panelProportions;
			panelProportions.relativeSize = glm::vec2(0.8f);
			mGUIManager.add(&mPanel, se::app::Anchor(), panelProportions);
		};

		/** Class destructor */
		~GameMenuView()
		{
			mGUIManager.remove(&mPanel);
		};
	};

}

#endif		// GAME_MENU_VIEW_H
