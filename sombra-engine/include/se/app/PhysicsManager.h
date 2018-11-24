#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <map>
#include <memory>
#include "../physics/RigidBody.h"
#include "../physics/PhysicsEngine.h"

namespace se::app {

	struct Entity;


	/**
	 * Class PhysicsManager, it's an Manager used for storing and updating
	 * the Entities' phisics data
	 */
	class PhysicsManager
	{
	private:	// Nested types
		using RigidBodyUPtr = std::unique_ptr<physics::RigidBody>;

	private:	// Attributes
		/** Maps the Entries added to the PhysicsManager and its physics data */
		std::map<Entity*, RigidBodyUPtr> mEntityMap;

		/** The Engine used for updating the data of the PhysicsEntities */
		physics::PhysicsEngine& mPhysicsEngine;

	public:		// Functions
		/** Creates a new PhysicsManager */
		PhysicsManager(physics::PhysicsEngine& physicsEngine) :
			mPhysicsEngine(physicsEngine) {};

		/** Adds the given Entity to the PhysicsManager and its physics data
		 * to the manager
		 *
		 * @param	rigidBody a pointer to the RigidBody to add to the
		 *			PhysicsManager
		 * @param	rigidBody the physics data of the Entity
		 * @note	The RigidBody initial data is overrided by the Entity one */
		void addEntity(Entity* entity, RigidBodyUPtr rigidBody);

		/** Removes the given Entity from the PhysicsManager so it won't
		 * longer be updated
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			PhysicsManager */
		void removeEntity(Entity* entity);

		/** Integrates the RigidBodies data of the entities
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void doDynamics(float delta);

		/** Solves the Constraints between the RigidBodies of the entities
		 *
		 * @param	delta the elapsed time since the last update in seconds */
		void doConstraints(float delta);
	};

}

#endif	// PHYSICS_MANAGER_H
