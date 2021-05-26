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

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&AnimationSystem::onNewAComponent, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&AnimationSystem::onRemoveAComponent, entity, mask); };

		/** Updates the Entities' animations */
		virtual void update() override;
	private:
		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		void onNewAComponent(
			Entity entity, AnimationComponent* animationComponent
		);

		/** Function called when an AnimationComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the AnimationComponent
		 * @param	animationComponent a pointer to the AnimationComponent that
		 *			is going to be removed */
		void onRemoveAComponent(
			Entity entity, AnimationComponent* animationComponent
		);
	};

}

#endif		// ANIMATION_SYSTEM_H
