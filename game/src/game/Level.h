#ifndef LEVEL_H
#define LEVEL_H

#include <se/app/events/EventManager.h>
#include <se/app/events/KeyEvent.h>

#include <se/graphics/2D/Layer2D.h>
#include <se/graphics/2D/Renderable2D.h>
#include <se/graphics/2D/RenderableText.h>

#include <se/app/Entity.h>
#include <se/audio/Buffer.h>
#include <se/physics/forces/Force.h>
#include <se/physics/constraints/Constraint.h>
#include <se/animation/IAnimator.h>

#include "IGameScreen.h"
#include "PlayerController.h"

namespace game {

	/**
	 * Class Level, TODO:
	 */
	class Level : public IGameScreen, public se::app::IEventListener
	{
	private:	// Nested Types
		using EntityUPtr = std::unique_ptr<se::app::Entity>;

	private:	// Attributes
		static constexpr unsigned int kNumCubes		= 50;
		static constexpr float kFOV					= 60.0f;
		static constexpr float kZNear				= 0.1f;
		static constexpr float kZFar				= 2000.0f;

		std::vector<EntityUPtr> mEntities;
		se::app::Entity* mPlayerEntity;
		PlayerController* mPlayerController;
		std::vector<se::audio::Buffer> mBuffers;
		std::vector<se::physics::Force*> mForces;
		std::vector<se::physics::Constraint*> mConstraints;
		std::vector<se::animation::IAnimator*> mAnimators;
		se::graphics::Renderable2D *mLogoTexture, *mReticleTexture;
		se::graphics::RenderableText* mPickText;

	public:		// Functions
		/** Creates a new Level and loads all the needed resources into
		 * the GameData
		 *
		 * @param	gameData a reference to the GameData where we are going to
		 *			load the Level menu */
		Level(GameData& gameData);

		/** Class destructor, it clears all the used resources from the
		 * GameData */
		virtual ~Level();

		/** Used for updating the Level at the same time than the Game does
		 * it
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void update(float deltaTime) override;

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
