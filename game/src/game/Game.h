#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>
#include <string>
#include <se/app/Application.h>
#include <se/graphics/2D/Layer2D.h>
#include <se/graphics/2D/Renderable2D.h>
#include <se/audio/Buffer.h>
#include <se/physics/forces/Force.h>
#include <se/physics/constraints/Constraint.h>
#include <se/animation/IAnimator.h>

namespace game {

	/**
	 * Class Engine
	 */
	class Game : public se::app::Application
	{
	private:	// Constants
		static const std::string kTitle;
		static const unsigned int kWidth;
		static const unsigned int kHeight;
		static const float kUpdateTime;
		static const unsigned int kNumCubes;
		static const float kFOV;
		static const float kZNear;
		static const float kZFar;

	private:	// Attributes
		se::graphics::Layer2D mLayer2D;
		std::vector<se::graphics::Renderable2D> mRenderable2Ds;
		std::vector<se::audio::Buffer> mBuffers;
		std::vector<se::physics::Force*> mForces;
		std::vector<se::physics::Constraint*> mConstraints;
		std::vector<se::animation::IAnimator*> mAnimators;

	public:		// Functions
		/** Creates a new Game */
		Game();

		/** Class destructor */
		~Game();
	private:
		/**
		 * Function used to initialize the application's entities data
		 */
		void init() override;

		/**
		 * Function used to remove the application's entities data
		 */
		void end() override;
	};

}

#endif		// GAME_H
