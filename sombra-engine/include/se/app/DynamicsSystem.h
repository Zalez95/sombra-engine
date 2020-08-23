#ifndef DYNAMICS_SYSTEM_H
#define DYNAMICS_SYSTEM_H

#include "ISystem.h"

namespace se::app {

	class Application;


	/**
	 * Class DynamicsSystem, it's a System for updating the Entities' physics
	 * data
	 */
	class DynamicsSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the PhysicsEngine used for updating
		 * Entities */
		Application& mApplication;

	public:		// Functions
		/** Creates a new DynamicsSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		DynamicsSystem(Application& application);

		/** Class destructor */
		~DynamicsSystem();

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Integrates the RigidBodies data of the entities */
		virtual void update() override;
	};

}

#endif	// DYNAMICS_SYSTEM_H
