#ifndef I_GAME_SCREEN_H
#define I_GAME_SCREEN_H

namespace game {

	struct GameData;


	/**
	 * class IGameScreen, it's the interface that every game screen should
	 * implement to be loaded or unloaded from the GameData
	 */
	class IGameScreen
	{
	protected:	// Attributes
		/** A reference to the GameData where we want to load the data */
		GameData& mGameData;

	public:		// Functions
		/** Creates a new IGameScreen, loading all its needed resources into
		 * the GameData
		 *
		 * @param	gameData a reference to the GameData where we want to load
		 *			the data */
		IGameScreen(GameData& gameData) : mGameData(gameData) {};

		/** Class destructor, it clears all the resources from the GameData*/
		virtual ~IGameScreen() = default;

		/** Used for updating the GameScreen at the same time than the Game does
		 * it
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void update(float deltaTime) = 0;
	};

}

#endif		// I_GAME_SCREEN_H
