#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <map>
#include <vector>
#include "../collision/CollisionDetector.h"
#include "forces/ForceManager.h"
#include "constraints/ConstraintBounds.h"
#include "constraints/NormalConstraint.h"
#include "constraints/ConstraintManager.h"

namespace physics {

	class PhysicsEntity;
	class Collider;


	/**
	 * Class PhysicsEngine, it's used to update the position and orientation
	 * of all the PhysicsEntities
	 */
	class PhysicsEngine
	{
	private:	// Constants
		/** The CounstraintBounds shared by all the NormalConstraints */
		static const ConstraintBounds CONSTRAINT_BOUNDS;

		/** The velocity of the constraint resolution process */
		static const float CONSTRAINT_BETA;

	private:	// Attributes
		/** The ForceManager of the PhysicsEngine. It's used to store the
		 * relationships between the PhysicsEntities and the Forces and
		 * applying them */
		ForceManager mForceManager;

		/** The CollisionDetector used by the PhysicsEngine to detect all the
		 * collisions */
		collision::CollisionDetector mCollisionDetector;

		/** The ConstraintManager of the PhysicsEngine. We will delegate all
		 * the constraint resolution to it */
		ConstraintManager mConstraintManager;

		/** All the PhysicsEntities that must be updated */
		std::vector<PhysicsEntity*> mPhysicsEntities;

		/** Maps each Collider with the Entity that holds it */
		std::map<const collision::Collider*, PhysicsEntity*> mColliderEntityMap;

		/** The NormalConstraints generated as a consecuence of the
		 * PhysicsEntities collisions */
		std::map<collision::Contact*, NormalConstraint> mContactConstraints;

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

		/** @return	the ForceManager of the PhysicsEngine */
		inline ForceManager* getForceManager() { return &mForceManager; };

		/** @return	the ConstraintManager of the PhysicsEngine */
		inline ConstraintManager* getConstraintManager()
		{ return &mConstraintManager; };
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
