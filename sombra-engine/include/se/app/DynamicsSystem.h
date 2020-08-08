#ifndef DYNAMICS_SYSTEM_H
#define DYNAMICS_SYSTEM_H

#include "../physics/PhysicsEngine.h"
#include "ISystem.h"

namespace se::app {

	/**
	 * Class DynamicsSystem, it's a System for updating the Entities' physics
	 * data
	 */
	class DynamicsSystem : public ISystem
	{
	private:	// Attributes
		/** The Engine used for updating the data of the PhysicsEntities */
		physics::PhysicsEngine& mPhysicsEngine;

	public:		// Functions
		/** Creates a new DynamicsSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	physicsEngine a reference to the PhysicsEngine used
		 *			by the DynamicsSystem */
		DynamicsSystem(
			EntityDatabase& entityDatabase,
			physics::PhysicsEngine& physicsEngine
		);

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
