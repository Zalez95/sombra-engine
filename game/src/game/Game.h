#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>
#include <string>
#include <fe/app/Application.h>
#include <fe/graphics/2D/Layer2D.h>
#include <fe/graphics/2D/Renderable2D.h>

namespace game {

	/**
	 * Class Engine
	 */
	class Game : public fe::app::Application
	{
	private:    // Constants
		static const std::string sTitle;
		static const unsigned int sWidth;
		static const unsigned int sHeight;
		static const float sUpdateTime;
		static const unsigned int sNumCubes;

	private:	// Attributes
		fe::graphics::Layer2D mLayer2D;
		std::vector<fe::graphics::Renderable2D> mRenderable2Ds;

	public:		// Functions
		/** Creates a new Game */
		Game();

		/** Class destructor */
		~Game();
	private:
		/**
		 * Function used to initialize the application's entities data
		 */
		virtual void init();

		/**
		 * Function used to remove the application's entities data
		 */
		virtual void end();
	};

}

#endif		// GAME_H
