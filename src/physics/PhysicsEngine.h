#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <vector>
#include "forces/ForceManager.h"
#include "collision/CollisionDetector.h"
#include "collision/CollisionResolver.h"

namespace physics {

	class PhysicsEntity;
	

	/**
	 * Class PhysicsEngine, it's used to update the position and orientation
	 * of all the PhysicsEntities
	 */
	class PhysicsEngine
	{
	private:	// Attributes
		/** All the PhysicsEntities that must be updated */
		std::vector<PhysicsEntity*> mPhysicsEntities;

		/** The force Manager used to store the relationships between the
		 * PhysicsEntities and the Forces */
		ForceManager mForceManager;

		/** We will delegate all the collision data calculation to this
         * CollisionDetector */
		CollisionDetector mCollisionDetector;

		/** We will delegate all the collision resolution to this
         * CollisionResolver */
		CollisionResolver mCollisionResolver;

	public:		// Functions
		/** Creates a new PhysicsEngine */
		PhysicsEngine() {};

		/** Class destructor */
		~PhysicsEngine() {};
		
		/** Updates the PhysicsEntities added to the System each time you
		 * call this function
		 *
		 * @param delta the elapsed time since the last Update call */
		void update(float delta);

		/** Adds the given PhysicsEntity to the PhysicsEngine so it will
		 * be updated in each Update call
		 * 
		 * @param	component a pointer to the PhysicsEntity that we want
		 *			to update */
		void addPhysicsEntity(PhysicsEntity* component);

		/** Removes the given PhysicsEntity from the PhysicsEngine so it
		 * won't longer be updated
		 * 
		 * @param	component a pointer to the PhysicsEntity that we want
		 *			to update
		 * @note	you must call this function before deleting the
		 *			PhysicsEntity */
		void removePhysicsEntity(PhysicsEntity* entity);
	private:
		/** Updates the positions of the PhysicsEntities added to the System
		 * 
		 * @param	delta the elapsed time since the last Update of the
		 *			PhysicsEngine */
		void integrate(float delta);

		/** Collision detections of the PhysicsEntities
		 *
		 * @param	delta the elapsed time since the last Update of the
		 *			PhysicsEngine */
		void collide(float delta);
	};

}

#endif		// PHYSICS_SYSTEM_H
