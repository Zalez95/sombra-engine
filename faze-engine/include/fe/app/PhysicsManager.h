#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <memory>
#include <map>
#include "Entity.h"
#include "../physics/PhysicsEntity.h"
#include "../physics/PhysicsEngine.h"

namespace fe { namespace app {

	struct Entity;


	/**
	 * Class PhysicsManager, it's an Manager used for storing and updating
	 * the Entities' phisics data
	 */
	class PhysicsManager
	{
	private:	// Nested types
		typedef std::unique_ptr<physics::PhysicsEntity> PhysicsEntityUPtr;

	private:	// Attributes
		/** Maps the Entries added to the PhysicsManager and its physics
		 * data */
		std::map<Entity*, PhysicsEntityUPtr> mEntityMap;

		/** The Engine used for updating the data of the PhysicsEntities */
		physics::PhysicsEngine& mPhysicsEngine;

	public:		// Functions
		/** Creates a new PhysicsManager */
		PhysicsManager(physics::PhysicsEngine& physicsEngine) :
			mPhysicsEngine(physicsEngine) {};

		/** Class destructor */
		~PhysicsManager() {};

		/** Adds the given Entity to the PhysicsManager and its physics data
		 * to the manager
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			PhysicsManager
		 * @param	physicsEntity the physics data of the Entity
		 * @note	The rigid body initial data is overrided by the entity
		 *			one */
		void addEntity(Entity* entity, PhysicsEntityUPtr physicsEntity);

		/** Removes the given Entity from the PhysicsManager so it won't
		 * longer be updated
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			PhysicsManager */
		void removeEntity(Entity* entity);

		/** Updates the physics data of the entities
		 *
		 * @param	delta the elapsed time since the last update */
		void update(float delta);
	};

}}

#endif	// PHYSICS_MANAGER_H
