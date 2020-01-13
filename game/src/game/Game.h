#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>
#include <string>
#include <se/app/Application.h>
#include <se/app/Entity.h>
#include <se/graphics/2D/Layer2D.h>
#include <se/graphics/2D/Renderable2D.h>
#include <se/graphics/2D/RenderableText.h>
#include <se/audio/Buffer.h>
#include <se/physics/forces/Force.h>
#include <se/physics/constraints/Constraint.h>
#include <se/animation/IAnimator.h>
#include "GameCommands.h"

namespace game {

	/**
	 * Class Engine
	 */
	class Game : public se::app::Application
	{
	private:	// Nested Types
		using EntityUPtr = std::unique_ptr<se::app::Entity>;

	private:	// Constants
		static constexpr char kTitle[]			= "< SOMBRA >";
		static constexpr unsigned int kWidth	= 1280;
		static constexpr unsigned int kHeight	= 720;
		static constexpr float kUpdateTime		= 0.016f;
		static constexpr unsigned int kNumCubes	= 50;
		static constexpr float kFOV				= 60.0f;
		static constexpr float kZNear			= 0.1f;
		static constexpr float kZFar			= 2000.0f;
		static constexpr float kRunSpeed		= 2.5f;
		static constexpr float kJumpSpeed		= 3.0f;
		static constexpr float kMouseSpeed		= 100.0f;
		static constexpr float kPitchLimit		= 0.05f;

	private:	// Attributes
		se::graphics::Layer2D mLayer2D;

		std::vector<EntityUPtr> mEntities;
		se::app::Entity* mPlayer;
		InputTransforms mPlayerInput;
		std::vector<se::graphics::Renderable2D> mRenderable2Ds;
		std::vector<se::graphics::RenderableText> mRenderableTexts;
		se::graphics::RenderableText* mFPSText;
		std::vector<se::audio::Buffer> mBuffers;
		std::vector<se::physics::Force*> mForces;
		std::vector<se::physics::Constraint*> mConstraints;
		std::vector<se::animation::IAnimator*> mAnimators;

	public:		// Functions
		/** Creates a new Game */
		Game();

		/** Class destructor */
		virtual ~Game();

		/** Function used start the game */
		virtual void start() override;

		/** Function used to stop the game */
		virtual void stop() override;
	private:
		/** Updates the game managers and systems each main loop
		 * iteration
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void onUpdate(float deltaTime) override;
	};

}

#endif		// GAME_H
