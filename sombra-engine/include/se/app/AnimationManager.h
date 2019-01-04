#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <map>
#include <memory>
#include "../animation/AnimationSystem.h"

namespace se::app {

	struct Entity;


	/**
	 * Class AnimationManager, it's a Manager used for storing and updating the
	 * Entities' animation data
	 */
	class AnimationManager
	{
	private:	// Nested types
		using AnimationUPtr = std::unique_ptr<animation::Animation>;

	private:	// Attributes
		/** The AnimationSystem used for updating the animation of the
		 * Entities */
		animation::AnimationSystem& mAnimationSystem;

		/** All the Animation Entities added to the AnimationManager */
		std::map<Entity*, AnimationUPtr> mAnimationEntities;

	public:		// Functions
		/** Creates a new AnimationManager
		 *
		 * @param	animationSystem a reference to the AnimationSystem used by
		 * 			the AnimationManager to update the entities' animations */
		AnimationManager(animation::AnimationSystem& animationSystem) :
			mAnimationSystem(animationSystem) {};

		/** Adds the given Entity to the AnimationManager
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			AnimationManager
		 * @param	animation a pointer to the Animation to add to the
		 *			AnimationManager */
		void addEntity(Entity* entity, AnimationUPtr animation);

		/** Removes the given Entity from the AnimationManager
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			AnimationManager */
		void removeEntity(Entity* entity);

		/** Updates the Entities' animations
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void update(float delta);
	};

}

#endif		// ANIMATION_MANAGER_H
