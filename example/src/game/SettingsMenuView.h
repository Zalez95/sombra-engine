#ifndef SETTINGS_MENU_VIEW_H
#define SETTINGS_MENU_VIEW_H

#include <vector>
#include <se/app/Repository.h>
#include <se/app/gui/Panel.h>
#include <se/app/gui/Label.h>
#include <se/app/gui/Button.h>
#include <se/app/gui/Rectangle.h>
#include <se/app/gui/GUIManager.h>
#include "SettingsMenuController.h"
#include "Game.h"

namespace game {

	/**
	 * Class SettingsMenuView, it holds and configures all the IComponents used
	 * for drawing the settings menu screen
	 */
	class SettingsMenuView
	{
	public:		// Nested types
		/** The Label to set */
		enum class SelectionLabel { Yes, No, Windowed, FullScreen };

	private:	// Attributes
		/** The the Game that holds the GUIManager used for retrieving input
		 * events */
		Game& mGame;

		/** The controller that will handle the user input */
		SettingsMenuController& mController;

		/** The Labels of the SettingsMenu */
		std::vector<se::app::Label> mLabels;

		/** The different Buttons of the SettingsMenu */
		std::vector<se::app::Button> mButtons;

		/** Pointers to the labels to change its text dynamically */
		se::app::Label *mSelectedWindowLabel, *mSelectedVSyncLabel;

		/** The Panel that will Hold all the elements of the SettingsMenu */
		se::app::Panel mPanel;

	public:		// Functions
		/** Creates a new SettingsMenuView
		 *
		 * @param	game the Game that holds the GUIManager
		 * @param	controller the SettingsMenuController that will handle the
		 *			user input */
		SettingsMenuView(Game& game, SettingsMenuController& controller) :
			mGame(game), mController(controller),
			mSelectedWindowLabel(nullptr), mSelectedVSyncLabel(nullptr),
			mPanel(&game.getGUIManager())
		{
			auto arial = game.getRepository().findByName<se::graphics::Font>("arial");

			mLabels.reserve(2 + 2 * 4);
			mButtons.reserve(1 + 2 * 2);

			auto& titleLabel = mLabels.emplace_back(&game.getGUIManager());
			titleLabel.setFont(arial);
			titleLabel.setCharacterSize({ 32, 32 });
			titleLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			titleLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			titleLabel.setColor(glm::vec4(1.0f));
			titleLabel.setText("SETTINGS");
			se::app::Anchor titleLabelAnchor;
			titleLabelAnchor.relativePosition = { 0.5f, 0.1f };
			se::app::Proportions titleLabelProportions;
			titleLabelProportions.relativeSize = { 0.25f, 0.1f };
			mPanel.add(&titleLabel, titleLabelAnchor, titleLabelProportions);

			addParameter(
				"Window mode", 0.25f,
				[this]() { mController.onWindow(SettingsMenuController::ButtonOption::Left); },
				[this]() { mController.onWindow(SettingsMenuController::ButtonOption::Right); }
			);
			mSelectedWindowLabel = &mLabels.back();
			addParameter(
				"VSync", 0.4f,
				[this]() { mController.onVSync(SettingsMenuController::ButtonOption::Left); },
				[this]() { mController.onVSync(SettingsMenuController::ButtonOption::Right); }
			);
			mSelectedVSyncLabel = &mLabels.back();

			auto& backLabel = mLabels.emplace_back(&game.getGUIManager());
			backLabel.setFont(arial);
			backLabel.setCharacterSize({ 24, 24 });
			backLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			backLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			backLabel.setColor(glm::vec4(1.0f));
			backLabel.setText("Back");

			auto& backButton = mButtons.emplace_back(&game.getGUIManager(), std::make_unique<se::app::Rectangle>());
			backButton.setColor({ 1.0f, 0.5f, 0.5f, 1.0f });
			backButton.setLabel(&backLabel);
			backButton.setAction([this]() { mController.onBack(); });
			se::app::Anchor backButtonAnchor;
			backButtonAnchor.relativePosition = { 0.1f, 0.85f };
			se::app::Proportions backButtonProportions;
			backButtonProportions.relativeSize = { 0.15f, 0.1f };
			mPanel.add(&backButton, backButtonAnchor, backButtonProportions);

			mPanel.setColor({ 0.153f, 0.275f, 0.392f, 1.0f });
			mGame.getGUIManager().add(&mPanel, se::app::Anchor(), se::app::Proportions());
		};

		/** Class destructor */
		~SettingsMenuView()
		{
			mGame.getGUIManager().remove(&mPanel);
		};

		/** Sets the Windowed selection Label
		 *
		 * @param	selection the Label to set */
		void setWindowed(SelectionLabel selection)
		{
			mSelectedWindowLabel->setText( getLabel(selection) );
		};

		/** Sets the VSync selection Label
		 *
		 * @param	selection the Label to set */
		void setVSync(SelectionLabel selection)
		{
			mSelectedVSyncLabel->setText( getLabel(selection) );
		};
	private:
		/** Adds a parameter to set in the settings menu
		 *
		 * @param	parameterName the name of the new parameter
		 * @param	yOffset the offset in the Y axis of the new parameter
		 * @param	actionL the action to do when the left button is pressed
		 * @param	actionR the action to do when the right button is pressed
		 * @note	this function will append 2 buttons at the end of
		 *			@see mButtons and 4 Labels at the end of @see mLabels, with
		 *			the last one being the one used for showing the parameter
		 *			value */
		void addParameter(
			const char* parameterName, float yOffset,
			const std::function<void()>& actionL,
			const std::function<void()>& actionR
		) {
			auto arial = mGame.getRepository().findByName<se::graphics::Font>("arial");

			auto& parameterLabel = mLabels.emplace_back(&mGame.getGUIManager());
			parameterLabel.setFont(arial);
			parameterLabel.setCharacterSize({ 24, 24 });
			parameterLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Left);
			parameterLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			parameterLabel.setColor(glm::vec4(1.0f));
			parameterLabel.setText(parameterName);
			se::app::Anchor parameterLabelAnchor;
			parameterLabelAnchor.origin = se::app::Anchor::Origin::TopLeft;
			parameterLabelAnchor.relativePosition = { 0.15f, yOffset };
			se::app::Proportions parameterLabelProportions;
			parameterLabelProportions.relativeSize = { 0.25f, 0.1f };
			mPanel.add(&parameterLabel, parameterLabelAnchor, parameterLabelProportions);

			auto& buttonLLabel = mLabels.emplace_back(&mGame.getGUIManager());
			buttonLLabel.setFont(arial);
			buttonLLabel.setCharacterSize({ 24, 70 });
			buttonLLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Left);
			buttonLLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			buttonLLabel.setColor(glm::vec4(1.0f));
			buttonLLabel.setText("<");

			auto& buttonRLabel = mLabels.emplace_back(&mGame.getGUIManager());
			buttonRLabel.setFont(arial);
			buttonRLabel.setCharacterSize({ 24, 70 });
			buttonRLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Right);
			buttonRLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			buttonRLabel.setColor(glm::vec4(1.0f));
			buttonRLabel.setText(">");

			auto& valueLabel = mLabels.emplace_back(&mGame.getGUIManager());
			valueLabel.setFont(arial);
			valueLabel.setCharacterSize({ 24, 24 });
			valueLabel.setHorizontalAlignment(se::app::Label::HorizontalAlignment::Center);
			valueLabel.setVerticalAlignment(se::app::Label::VerticalAlignment::Center);
			valueLabel.setColor(glm::vec4(1.0f));
			se::app::Anchor valueLabelAnchor;
			valueLabelAnchor.relativePosition = { 0.65f, yOffset + 0.05f };
			se::app::Proportions valueLabelProportions;
			valueLabelProportions.relativeSize = { 0.15f, 0.1f };
			mPanel.add(&valueLabel, valueLabelAnchor, valueLabelProportions);

			se::app::Proportions buttonLRProportions;
			buttonLRProportions.relativeSize = { 0.15f, 0.1f };

			auto& buttonL = mButtons.emplace_back(&mGame.getGUIManager(), std::make_unique<se::app::Rectangle>());
			buttonL.setColor({ 0.0f, 1.0f, 0.0f, 0.75f });
			buttonL.setLabel(&buttonLLabel, { 0.9f, 1.0f });
			buttonL.setAction(actionL);
			se::app::Anchor buttonLAnchor;
			buttonLAnchor.origin = se::app::Anchor::Origin::TopLeft;
			buttonLAnchor.relativePosition = { 0.5f, yOffset };
			mPanel.add(&buttonL, buttonLAnchor, buttonLRProportions);

			auto& buttonR = mButtons.emplace_back(&mGame.getGUIManager(), std::make_unique<se::app::Rectangle>());
			buttonR.setColor({ 1.0f, 0.0f, 0.0f, 0.75f });
			buttonR.setLabel(&buttonRLabel, { 0.9f, 1.0f });
			buttonR.setAction(actionR);
			se::app::Anchor buttonRAnchor;
			buttonRAnchor.origin = se::app::Anchor::Origin::TopLeft;
			buttonRAnchor.relativePosition = { 0.65f, yOffset };
			mPanel.add(&buttonR, buttonRAnchor, buttonLRProportions);
		}

		/** Returns the Label text for the given selection
		 *
		 * @param	selection the selected Label
		 * @return	the Label text */
		static constexpr const char* getLabel(SelectionLabel selection)
		{
			switch (selection) {
				case SelectionLabel::Windowed:		return "Windowed";
				case SelectionLabel::FullScreen:	return "FullScreen";
				case SelectionLabel::Yes:			return "Yes";
				default:							return "No";
			}
		};
	};

}

#endif		// SETTINGS_MENU_VIEW_H
