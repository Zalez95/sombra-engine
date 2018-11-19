#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>
#include <string>
#include <se/app/Application.h>
#include <se/graphics/2D/Layer2D.h>
#include <se/graphics/2D/Renderable2D.h>
#include <se/audio/Buffer.h>
#include <se/physics/Constraint.h>

namespace game {

	/**
	 * Class Engine
	 */
	class Game : public se::app::Application
	{
	private:	// Constants
		static const std::string sTitle;
		static const unsigned int sWidth;
		static const unsigned int sHeight;
		static const float sUpdateTime;
		static const unsigned int sNumCubes;

	private:	// Attributes
		se::graphics::Layer2D mLayer2D;
		std::vector<se::graphics::Renderable2D> mRenderable2Ds;
		se::audio::Buffer buffer1;
		se::physics::Constraint* constraint;

	public:		// Functions
		/** Creates a new Game */
		Game() : se::app::Application(sTitle, sWidth, sHeight, sUpdateTime) {};

		/** Class destructor */
		~Game() {};
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
