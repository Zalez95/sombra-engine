#ifndef I_GAME_SCREEN_H
#define I_GAME_SCREEN_H

namespace game {

	class Game;


	/**
	 * class IGameScreen, it's the interface that every game screen should
	 * implement to be loaded or unloaded from the Game
	 */
	class IGameScreen
	{
	protected:	// Attributes
		/** A reference to the Game where we want to load the data */
		Game& mGame;

	public:		// Functions
		/** Creates a new IGameScreen, loading all its needed resources into
		 * the Game
		 *
		 * @param	game a reference to the Game where we want to load
		 *			the data */
		IGameScreen(Game& game) : mGame(game) {};

		/** Class destructor, it clears all the resources from the Game */
		virtual ~IGameScreen() = default;
	};

}

#endif		// I_GAME_SCREEN_H
