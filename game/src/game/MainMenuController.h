#ifndef MAIN_MENU_CONTROLLER_H
#define MAIN_MENU_CONTROLLER_H

#include "IGameScreen.h"

namespace game {

	class MainMenuView;


	/**
	 * Class MainMenuController, it's used for loading the main menu. It also
	 * implements the logic of the MainMenuView
	 */
	class MainMenuController : public IGameScreen
	{
	private:	// Attributes
		/** A pointer to the View controlled */
		MainMenuView* mView;

	public:		// Functions
		/** Creates a new MainMenuController and loads all the needed
		 * resources into the GameData
		 *
		 * @param	game a reference to the Game where we are going to add
		 *			the Main menu */
		MainMenuController(Game& game);

		/** Class destructor, it clears all the used resources from the
		 * GameData */
		virtual ~MainMenuController();

		/** Used for updating the MainMenuController at the same time than the
		 * Game does it
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void update(float deltaTime) override;

		/** Implements the start Button action */
		void onStart();

		/** Implements the config Button action */
		void onConfig();

		/** Implements the quit Button action */
		void onQuit();
	};

}

#endif		// MAIN_MENU_CONTROLLER_H
