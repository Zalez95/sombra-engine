#ifndef GAME_H
#define GAME_H

#include <se/app/Application.h>
#include <se/graphics/2D/RenderableText.h>
#include <se/utils/StateMachine.h>
#include "IGameScreen.h"

namespace game {

	static constexpr char kTitle[]					= "< SOMBRA >";
	static constexpr unsigned int kWidth			= 1280;
	static constexpr unsigned int kHeight			= 720;
	static constexpr float kMinFDifference			= 0.00001f;
	static constexpr float kContactPrecision		= 0.0000001f;
	static constexpr float kContactSeparation		= 0.00001f;
	static constexpr std::size_t kMaxManifolds		= 128;
	static constexpr int kMaxRayCasterIterations	= 32;
	static constexpr float kUpdateTime				= 0.016f;


	/** Holds all the game systems and data */
	struct GameData
	{
		se::utils::TaskManager* taskManager;
		se::app::EventManager* eventManager;
		se::app::EntityDatabase* entityDatabase;
		se::utils::Repository* repository;

		se::window::WindowSystem* windowSystem;
		se::graphics::GraphicsEngine* graphicsEngine;
		se::physics::PhysicsEngine* physicsEngine;
		se::collision::CollisionWorld* collisionWorld;
		se::animation::AnimationEngine* animationEngine;
		se::audio::AudioEngine* audioEngine;

		se::app::InputSystem* inputSystem;
		se::app::CameraSystem* cameraSystem;
		se::app::AppRenderer* appRenderer;
		se::app::RMeshSystem* rMeshSystem;
		se::app::RTerrainSystem* rTerrainSystem;
		se::app::DynamicsSystem* dynamicsSystem;
		se::app::ConstraintsSystem* constraintsSystem;
		se::app::CollisionSystem* collisionSystem;
		se::app::AnimationSystem* animationSystem;
		se::app::AudioSystem* audioSystem;
		se::app::GUIManager* guiManager;

		se::utils::StateMachine* stateMachine;
		se::graphics::RenderableText* fpsText;
		std::vector<IGameScreen*> currentGameScreens;
	};


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
	 * Class Game, TODO:
	 */
	class Game : public se::app::Application
	{
	private:	// Attributes
		/** All the game systems and data */
		GameData mGameData;

		/** The transition table of the StateMachine of the Game*/
		std::vector<se::utils::StateMachine::Transition> mGameTransitions;

		float mAccumulatedTime;
		int mNumFrames;

	public:		// Functions
		/** Creates a new Game */
		Game();

		/** Class destructor */
		~Game();
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
