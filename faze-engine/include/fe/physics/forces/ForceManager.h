#ifndef FORCE_MANAGER_H
#define FORCE_MANAGER_H

#include <vector>

namespace fe { namespace physics {

	class PhysicsEntity;
	class Force;


	/**
	 * Class ForceManager
	 */
	class ForceManager
	{
	private:	// Nested types
		/** Struct EntityForce, it's used to pair PhysicsEntities with Forces
		 * in the ForceManager */
		struct EntityForce
		{
			/** The entity that we want to pair */
			PhysicsEntity* entity;

			/** The force that we want to pair with the entity */
			Force* force;

			/** Creates a new EntityForce
			 *
			 * @param	entity a pointer to the PhysicsEntity of EntityForce
			 * @param	force a pointer to the Force of EntityForce */
			EntityForce(PhysicsEntity* entity, Force* force) :
				entity(entity), force(force) {};

			/** Class destructor */
			~EntityForce() {};
		};

	private:	// Attributes
		/** The vector that pairs the entities with the forces */
		std::vector<EntityForce> mEntityForces;

	public:		// Forces
		/** Creates a new ForceManager */
		ForceManager() {};

		/** Class destructor */
		~ForceManager() {};

		/** Registers the given PhisicsEntity with the given Force, so the
		 * Entity will be affected by the Force
		 *
		 * @param	entity a pointer to the PhysicsEntity that we want to be
		 *			affected by the Force
		 * @param	force a pointer to the Force that we want to register */
		void addEntity(PhysicsEntity* entity, Force* force);

		/** Unregisters the given Entity and given Force so the Entity
		 * won't longer be affected by the Force
		 *
		 * @param	entity a pointer to the PhysicsEntity that we want to
		 *			unregister
		 * @param	force a pointer to the Force that we want to unregister */
		void removeEntity(PhysicsEntity* entity, Force* force);

		/** Applies the forces stored in the ForceManager */
		void applyForces();
	};

}}

#endif		// FORCE_MANAGER_H
