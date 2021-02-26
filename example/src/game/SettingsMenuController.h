#ifndef SETTINGS_MENU_CONTROLLER_H
#define SETTINGS_MENU_CONTROLLER_H

#include "IGameScreen.h"

namespace game {

	class SettingsMenuView;


	/**
	 * Class SettingsMenuController, it's used for loading the settings menu.
	 * It also implements the logic of the SettingsMenuView
	 */
	class SettingsMenuController : public IGameScreen
	{
	public:		// Nested types
		/** The Button pressed */
		enum class ButtonOption { Left, Right };

	private:	// Attributes
		/** A pointer to the View controlled */
		SettingsMenuView* mView;

	public:		// Functions
		/** Creates a new SettingsMenuController and loads all the needed
		 * resources into the GameData
		 *
		 * @param	game a reference to the Game where we are going to add
		 *			the Main menu */
		SettingsMenuController(Game& game);

		/** Class destructor, it clears all the used resources from the
		 * GameData */
		virtual ~SettingsMenuController();

		/** Implements the back Button action */
		void onBack();

		/** Implements the Window Buttons action */
		void onWindow(ButtonOption option);

		/** Implements the VSync Buttons action */
		void onVSync(ButtonOption option);
	};

}

#endif		// SETTINGS_MENU_CONTROLLER_H
