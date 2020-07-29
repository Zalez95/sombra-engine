#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <map>
#include <memory>
#include "../animation/AnimationNode.h"
#include "../animation/AnimationEngine.h"

namespace se::app {

	struct Entity;


	/**
	 * Class AnimationManager, it's a Manager used for storing and updating the
	 * Entities' animation data
	 */
	class AnimationManager
	{
	private:	// Nested types
		/** Struct NodeEntity, holds a pointer to the Entity and other metadata
		 * about it */
		struct NodeEntity
		{
			/** Apointer to the entity */
			Entity* entity;

			/** If the entity ows any AnimationNode or not */
			bool nodeOwner;
		};

	private:	// Attributes
		/** The AnimationEngine used for updating the animation of the
		 * Entities */
		animation::AnimationEngine& mAnimationEngine;

		/** The root AnimationNode node of the hierarchy of AnimationNodes to
		 * update */
		std::unique_ptr<animation::AnimationNode> mRootNode;

		/** Maps the AnimationNode with their respective Entities */
		std::map<animation::AnimationNode*, NodeEntity> mNodeEntities;

	public:		// Functions
		/** Creates a new AnimationManager
		 *
		 * @param	animationEngine a reference to the AnimationEngine used by
		 *			the AnimationManager to update the entities' animations */
		AnimationManager(animation::AnimationEngine& animationEngine);

		/** Adds the given Entity to the AnimationManager with the given
		 * AnimationNode, so when the AnimationNode or any of its parents is
		 * updated due to an animation the Entity will also be updated
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			AnimationManager
		 * @param	animationNode a pointer to the AnimationNode to add to the
		 *			AnimationManager. The animationNode must have been already
		 *			inserted into the AnimationManager
		 * @note	The Entity initial data is overridden by the AnimationNode
		 *			one */
		void addEntity(Entity* entity, animation::AnimationNode* animationNode);

		/** Adds the given Entity to the AnimationManager with the given
		 * AnimationNode, so when the AnimationNode or any of its parents is
		 * updated due to an animation the Entity will also be updated
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			AnimationManager
		 * @param	animationNode a pointer to the AnimationNode to add to the
		 *			AnimationManager
		 * @note	The Entity initial data is overridden by the AnimationNode
		 *			one */
		void addEntity(
			Entity* entity,
			std::unique_ptr<animation::AnimationNode> animationNode
		);

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
