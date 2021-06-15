#ifndef LEVEL_H
#define LEVEL_H

#include <se/app/events/EventManager.h>
#include <se/app/events/KeyEvent.h>
#include <se/graphics/2D/RenderableSprite.h>
#include <se/graphics/2D/RenderableText.h>
#include <se/app/Scene.h>
#include <se/audio/Buffer.h>
#include <se/physics/forces/Force.h>
#include <se/physics/constraints/Constraint.h>
#include <se/animation/IAnimator.h>
#include "IGameScreen.h"

namespace game {

	/**
	 * Class Level, it's the single level of the Example Game
	 */
	class Level : public IGameScreen, public se::app::IEventListener
	{
	private:	// Attributes
		static constexpr unsigned int kNumCubes		= 50;
		static constexpr float kFOV					= 60.0f;
		static constexpr float kZNear				= 0.1f;
		static constexpr float kZFar				= 2000.0f;

		Game& mGame;
		se::app::Scene mScene;
		se::app::Entity mPlayerEntity;
		se::graphics::RenderableSprite *mLogoTexture, *mReticleTexture;
		se::graphics::RenderableText* mPickText;

	public:		// Functions
		/** Creates a new Level and loads all the needed resources into
		 * the GameData
		 *
		 * @param	game a reference to the Game where we are going to
		 *			load the Level menu */
		Level(Game& game);

		/** Class destructor, it clears all the used resources from the
		 * GameData */
		virtual ~Level();

		/** @return	the Game that holds the Level */
		Game& getGame() { return mGame; };

		/** @return	the Scene of the Level */
		se::app::Scene& getScene() { return mScene; };

		/** @return	the player Entity of the Level */
		se::app::Entity getPlayer() const { return mPlayerEntity; };

		/** Notifies the Level of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const se::app::IEvent& event) override;

		/** Makes the Level handle the input or not
		 *
		 * @param	handle true if the Level should handle the input, false
		 *			otherwise */
		void setHandleInput(bool handle);
	private:
		/** Handles the given event
		 *
		 * @param	event the KeyEvent to handle */
		void onKeyEvent(const se::app::KeyEvent& event);
	};

}

#endif		// LEVEL_H
