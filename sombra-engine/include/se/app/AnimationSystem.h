#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include <memory>
#include "../animation/AnimationNode.h"
#include "../animation/AnimationEngine.h"
#include "ISystem.h"

namespace se::app {

	/**
	 * Class AnimationSystem, it's a System used for updating the
	 * Entities' animation data
	 */
	class AnimationSystem : public ISystem
	{
	private:	// Attributes
		/** The AnimationEngine used for updating the animation of the
		 * Entities */
		animation::AnimationEngine& mAnimationEngine;

		/** The root AnimationNode node of the hierarchy of AnimationNodes to
		 * update */
		std::unique_ptr<animation::AnimationNode> mRootNode;

	public:		// Functions
		/** Creates a new AnimationSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	animationEngine a reference to the AnimationEngine used by
		 *			the AnimationSystem to update the entities' animations */
		AnimationSystem(
			EntityDatabase& entityDatabase,
			animation::AnimationEngine& animationEngine
		);

		/** Class destructor */
		~AnimationSystem();

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity) override;

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity) override;

		/** Updates the Entities' animations */
		virtual void update() override;
	};

}

#endif		// ANIMATION_SYSTEM_H
