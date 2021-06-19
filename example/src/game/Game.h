#ifndef GAME_H
#define GAME_H

#include <se/app/Application.h>
#include <se/graphics/2D/RenderableText.h>
#include <se/utils/StateMachine.h>
#include "IGameScreen.h"

namespace game {

	static constexpr char kTitle[]							= "< SOMBRA >";
	static constexpr unsigned int kWidths[]					= { 1280, 1920 };
	static constexpr unsigned int kHeights[]				= { 720, 1080 };
	static constexpr std::size_t kMaxManifolds				= 128;
	static constexpr float kMinFDifference					= 0.00001f;
	static constexpr std::size_t kMaxCollisionIterations	= 128;
	static constexpr float kContactPrecision				= 0.0000001f;
	static constexpr float kContactSeparation				= 0.00001f;
	static constexpr int kMaxRayCasterIterations			= 32;
	static constexpr float kUpdateTime						= 0.016f;

	enum class GameState : se::utils::StateMachine::State
	{
		Start,
		MainMenu,
		SettingsMenu,
		Level,
		GameMenu,
		Stopped
	};


	enum class GameEvent : se::utils::StateMachine::Event
	{
		GoToMainMenu,
		GoToSettings,
		StartLevel,
		AddGameMenu,
		RemoveGameMenu,
		Quit
	};


	/**
	 * Class Game, it's an example game created using the SombraEngine
	 * Application class
	 */
	class Game : public se::app::Application
	{
	private:	// Attributes
		se::utils::StateMachine* mStateMachine;
		se::graphics::RenderableText* mFPSText;
		std::vector<IGameScreen*> mCurrentGameScreens;

		/** The transition table of the StateMachine of the Game*/
		std::vector<se::utils::StateMachine::Transition> mGameTransitions;

		float mAccumulatedTime;
		int mNumFrames;

	public:		// Functions
		/** Creates a new Game */
		Game();

		/** Class destructor */
		~Game();

		/** @return	the State Machine of the Game */
		se::utils::StateMachine& getStateMachine() { return *mStateMachine; };
	protected:
		/** Updates the Game managers and systems each main loop
		 * iteration
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void onUpdate(float deltaTime) override;
	};

}

#endif		// GAME_H
