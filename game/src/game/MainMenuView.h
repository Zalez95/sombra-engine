#ifndef MAIN_MENU_VIEW_H
#define MAIN_MENU_VIEW_H

#include <se/app/gui/Panel.h>
#include <se/app/gui/Label.h>
#include <se/app/gui/Button.h>
#include <se/app/gui/Rectangle.h>
#include <se/app/gui/GUIManager.h>
#include <se/graphics/2D/Layer2D.h>
#include <se/utils/Repository.h>
#include "MainMenuController.h"

namespace game {

	/**
	 * Class MainMenuView, it holds and configures all the IComponents used for
	 * drawing the main menu screen
	 */
	class MainMenuView
	{
	private:	// Attributes
		/** The Layer2D that will be used for drawing the GUI */
		se::graphics::Layer2D& mLayer2D;

		/** The GUIManager used for retrieving input events */
		se::app::GUIManager& mGUIManager;

		/** The controller that will handle the user input */
		MainMenuController& mController;

		/** The Panel that will Hold all the elements of the MainMenu */
		se::app::Panel mPanel;

		/** The Labels of the MainMenu */
		se::app::Label mTitleLabel, mVersionLabel;

		/** The different Buttons of the MainMenu */
		se::app::Button mStartButton, mConfigButton, mQuitButton;

		/** The Labels of the MainMenu Buttons */
		se::app::Label mStartLabel, mConfigLabel, mQuitLabel;

	public:		// Functions
		/** Creates a new MainMenuView
		 *
		 * @param	layer2D the Layer2D to use for rendering the GUI components
		 * @param	guiManager the GUIManager to use for retrieving input
		 *			events
		 * @param	controller the MainMenuController that will handle the user
		 *			input */
		MainMenuView(
			se::graphics::Layer2D& layer2D,
			se::app::GUIManager& guiManager,
			MainMenuController& controller
		) : mLayer2D(layer2D), mGUIManager(guiManager), mController(controller),
			mPanel(&mLayer2D), mTitleLabel(&mLayer2D), mVersionLabel(&mLayer2D),
			mStartButton(&mLayer2D, std::make_unique<se::app::Rectangle>()),
			mConfigButton(&mLayer2D, std::make_unique<se::app::Rectangle>()),
			mQuitButton(&mLayer2D, std::make_unique<se::app::Rectangle>()),
			mStartLabel(&mLayer2D), mConfigLabel(&mLayer2D),
			mQuitLabel(&mLayer2D)
		{
			using FontSPtr = std::shared_ptr<se::graphics::Font>;
			auto arial = se::utils::Repository<FontSPtr>::getInstance()
				.get([](FontSPtr font) { return font->name == "Arial"; });

			mTitleLabel.setText("SOMBRA");
			mTitleLabel.setFont(arial);
			mTitleLabel.setCharacterSize(glm::vec2(56.0f));
			mTitleLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mTitleLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mTitleLabel.setColor(glm::vec4(1.0f));
			se::app::Anchor titleLabelAnchor;
			titleLabelAnchor.relativePosition = { 0.5f, 0.25f };
			se::app::Proportions titleLabelProportions;
			titleLabelProportions.relativeSize = { 0.5f, 0.2f };
			mPanel.add(&mTitleLabel, titleLabelAnchor, titleLabelProportions);

			mVersionLabel.setText("Version");
			mVersionLabel.setFont(arial);
			mVersionLabel.setCharacterSize(glm::vec2(16.0f));
			mVersionLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mVersionLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mVersionLabel.setColor(glm::vec4(1.0f));
			se::app::Anchor configLabelAnchor;
			configLabelAnchor.origin = se::app::Anchor::Origin::BottomLeft;
			configLabelAnchor.relativePosition = { 0.0f, 1.0f };
			se::app::Proportions configLabelProportions;
			configLabelProportions.relativeSize = { 0.2f, 0.1f };
			mPanel.add(&mConfigLabel, configLabelAnchor, configLabelProportions);

			mStartLabel.setText("Start");
			mStartLabel.setFont(arial);
			mStartLabel.setCharacterSize(glm::vec2(24.0f));
			mStartLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mStartLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mStartLabel.setColor(glm::vec4(1.0f));
			mStartButton.setLabel(&mStartLabel);

			mConfigLabel.setText("Configuration");
			mConfigLabel.setFont(arial);
			mConfigLabel.setCharacterSize(glm::vec2(24.0f));
			mConfigLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mConfigLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mConfigLabel.setColor(glm::vec4(1.0f));
			mConfigButton.setLabel(&mConfigLabel);

			mQuitLabel.setText("Quit");
			mQuitLabel.setFont(arial);
			mQuitLabel.setCharacterSize(glm::vec2(24.0f));
			mQuitLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			mQuitLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			mQuitLabel.setColor(glm::vec4(1.0f));
			mQuitButton.setLabel(&mQuitLabel);

			mStartButton.setColor({ 0.7f, 0.7f, 0.7f, 0.5f });
			mStartButton.setAction([this]() { mController.onStart(); });
			se::app::Anchor startButtonAnchor;
			startButtonAnchor.relativePosition = { 0.5f, 0.5f };
			se::app::Proportions startButtonProportions;
			startButtonProportions.relativeSize = { 0.5f, 0.1f };
			mPanel.add(&mStartButton, startButtonAnchor, startButtonProportions);

			mConfigButton.setColor({ 0.7f, 0.7f, 0.7f, 0.5f });
			mConfigButton.setAction([this]() { mController.onConfig(); });
			se::app::Anchor configButtonAnchor;
			configButtonAnchor.relativePosition = { 0.5f, 0.65 };
			se::app::Proportions configButtonProportions;
			configButtonProportions.relativeSize = { 0.5f, 0.1f };
			mPanel.add(&mConfigButton, configButtonAnchor, configButtonProportions);

			mQuitButton.setColor({ 0.7f, 0.7f, 0.7f, 0.5f });
			mQuitButton.setAction([this]() { mController.onQuit(); });
			se::app::Anchor quitButtonAnchor;
			quitButtonAnchor.relativePosition = { 0.5f, 0.8f };
			se::app::Proportions quitButtonProportions;
			quitButtonProportions.relativeSize = { 0.5f, 0.1f };
			mPanel.add(&mQuitButton, quitButtonAnchor, quitButtonProportions);

			mPanel.setColor({ 0.153f, 0.275f, 0.392f, 1.0f });
			mGUIManager.add(&mPanel, se::app::Anchor(), se::app::Proportions());
		};

		/** Class destructor */
		~MainMenuView()
		{
			mGUIManager.remove(&mPanel);
		};
	};

}

#endif		// MAIN_MENU_VIEW_H
