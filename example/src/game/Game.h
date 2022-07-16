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
	static constexpr float kMotionBias						= 0.1f;
	const se::physics::AABB kPhysicsWorldAABB				= { glm::vec3(-1000.0f), glm::vec3( 1000.0f) };
	static constexpr std::size_t kMaxCollidingRBs			= 128;
	static constexpr float kCoarseCollisionEpsilon			= 0.0001f;
	static constexpr std::size_t kMaxCollisionIterations	= 128;
	static constexpr float kMinFDifference					= 0.00001f;
	static constexpr float kContactPrecision				= 0.0000001f;
	static constexpr float kContactSeparation				= 0.00001f;
	static constexpr float kRaycastPrecision				= 0.0000001f;
	static constexpr float kCollisionBeta					= 0.1f;
	static constexpr float kCollisionRestitutionFactor		= 0.2f;
	static constexpr float kCollisionSlopPenetration		= 0.005f;
	static constexpr float kCollisionSlopRestitution		= 0.5f;
	static constexpr float kFrictionGravityAcceleration		= 9.8f;
	static constexpr std::size_t kNumSubsteps				= 4;
	static constexpr std::size_t kMaxConstraintIterations	= 1;
	static constexpr std::size_t kNumPhysicsThreads			= 4;
	static constexpr std::size_t kAudioDeviceId				= 0;
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
		 * @copydoc se::app::Application::onUpdate(float, float) */
		virtual void onUpdate(float deltaTime, float timeSinceStart) override;

		/** Draws to screen
		 * @copydoc se::app::Application::onRender(float, float) */
		virtual void onRender(float deltaTime, float timeSinceStart) override;
	};

}

#endif		// GAME_H
