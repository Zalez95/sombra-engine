#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include <memory>
#include "../animation/AnimationNode.h"
#include "ISystem.h"

namespace se::app {

	class Application;


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
