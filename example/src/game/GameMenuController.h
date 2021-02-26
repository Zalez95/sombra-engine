#ifndef GAME_MENU_CONTROLLER_H
#define GAME_MENU_CONTROLLER_H

#include "IGameScreen.h"

namespace game {

	class GameMenuView;


	/**
	 * Class GameMenuController, it's used for loading the game menu. It also
	 * implements the logic of the GameMenuView
	 */
	class GameMenuController : public IGameScreen
	{
	private:	// Attributes
		/** A pointer to the View controlled */
		GameMenuView* mView;

	public:		// Functions
		/** Creates a new GameMenuController and loads all the needed
		 * resources into the GameData
		 *
		 * @param	game a reference to the Game where we are going to add
		 *			the Main menu */
		GameMenuController(Game& game);

		/** Class destructor, it clears all the used resources from the
		 * GameData */
		virtual ~GameMenuController();

		/** Implements the back Button action */
		void onBack();

		/** Implements the quit Button action */
		void onQuit();
	};

}

#endif		// GAME_MENU_CONTROLLER_H
