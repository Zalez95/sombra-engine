#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include <memory>
#include "../animation/AnimationNode.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class AnimationComponent;


	/**
	 * Class AnimationSystem, it's a System used for updating the
	 * Entities' animation data
	 */
	class AnimationSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the AnimationEngine used for updating
		 * Entities' animations */
		Application& mApplication;

	public:		// Functions
		/** Creates a new AnimationSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		AnimationSystem(Application& application);

		/** Class destructor */
		~AnimationSystem();

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** Updates the Entities' animations */
		virtual void update() override;
	private:
		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity
		 * @param	animationComponent a pointer to the new AnimationComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewAComponent(
			Entity entity, AnimationComponent* animationComponent,
			EntityDatabase::Query& query
		);

		/** Function called when an AnimationComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the AnimationComponent
		 * @param	animationComponent a pointer to the AnimationComponent that
		 *			is going to be removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveAComponent(
			Entity entity, AnimationComponent* animationComponent,
			EntityDatabase::Query& query
		);
	};

}

#endif		// ANIMATION_SYSTEM_H
