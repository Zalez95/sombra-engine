#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <vector>
#include <unordered_map>
#include "forces/ForceManager.h"
#include "collision/CoarseCollisionDetector.h"
#include "collision/FineCollisionDetector.h"
#include "CollisionResolver.h"

namespace physics {

	class PhysicsEntity;
	class Collider;


	/**
	 * Class PhysicsEngine, it's used to update the position and orientation
	 * of all the PhysicsEntities
	 */
	class PhysicsEngine
	{
	private:	// Attributes
		/** All the PhysicsEntities that must be updated */
		std::vector<PhysicsEntity*> mPhysicsEntities;

		/** Maps each Collider with the Entity that holds it */
		std::unordered_map<const Collider*, PhysicsEntity*> mColliderEntityMap;

		/** The ForceManager of the PhysicsEngine. It's used to store the
		 * relationships between the PhysicsEntities and the Forces and
		 * applying them */
		ForceManager mForceManager;

		/** The CoarseCollisionDetector of the PhysicsEngine. We will use it
		 * to filter some of the PhysiscsEntities' Colliders during the
		 * collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the PhysicsEngine. We will delegate
		 * all the collision data calculation to it */
		FineCollisionDetector mFineCollisionDetector;

		/** The CollisionResolver of the PhysicsEngine. We will delegate all
		 * the collision response calculation to it */
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
		 * @param	entity a pointer to the PhysicsEntity that we want
		 *			to update */
		void addPhysicsEntity(PhysicsEntity* entity);

		/** Removes the given PhysicsEntity from the PhysicsEngine so it
		 * won't longer be updated
		 * 
		 * @param	entity a pointer to the PhysicsEntity that we want
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
