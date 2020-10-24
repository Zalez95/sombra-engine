#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "forces/ForceManager.h"
#include "constraints/ConstraintManager.h"
#include "RigidBody.h"

namespace se::physics {

	/**
	 * Class PhysicsEngine, it's used to update the position and orientation
	 * of all the RigidBodies
	 */
	class PhysicsEngine
	{
	private:	// Attributes
		/** The base bias value used for updating the RigidBodies' motion
		 * value */
		const float mBaseBias;

		/** The ForceManager of the PhysicsEngine. It's used to store the
		 * relationships between the RigidBodies and the Forces and
		 * applying them */
		ForceManager mForceManager;

		/** The ConstraintManager of the PhysicsEngine. We will delegate all
		 * the constraint resolution to it */
		ConstraintManager mConstraintManager;

		/** All the RigidBodies that must be updated */
		std::vector<RigidBody*> mRigidBodies;

	public:		// Functions
		/** Creates a new PhysicsEngine
		 *
		 * @param	baseBias the bias value used for updating the RigidBodies'
		 *			motion value */
		PhysicsEngine(float baseBias) : mBaseBias(baseBias) {};

		/** @return	the ForceManager of the PhysicsEngine */
		ForceManager& getForceManager() { return mForceManager; };

		/** @return	the ConstraintManager of the PhysicsEngine */
		ConstraintManager& getConstraintManager()
		{ return mConstraintManager; };

		/** Adds the given RigidBody to the PhysicsEngine so it will be updated
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to
		 *			update */
		void addRigidBody(RigidBody* rigidBody);

		/** Removes the given RigidBody from the PhysicsEngine so it won't
		 * longer be updated
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to update
		 * @note	the Forces and Constraints that references the RigidBody
		 *			will also be removed from the PhysicsEngine */
		void removeRigidBody(RigidBody* rigidBody);

		/** Resets the Integrated and ConstraintsSolved state of all the
		 * RigidBodies.
		 *
		 * @note	the state should be reset before integrating or solving the
		 *			constraints of the RigidBodies */
		void resetRigidBodiesState();

		/** Updates the positions of the RigidBodies added to the System
		 *
		 * @param	deltaTime the elapsed time since the last integration of
		 *			the RigidBodies in seconds */
		void integrate(float deltaTime);

		/** Updates the ConstraintManager to solve the Constraints between
		 * the RigidBodies
		 *
		 * @param	deltaTime the elapsed time since the last constraints
		 *			resolution in seconds
		 * @note	the constraints should be solved after integrating the
		 *			position of the RigidBodies */
		void solveConstraints(float deltaTime);

		/** Checks if any of the RigidBodies are stopped, and if it's the case,
		 * it sets them to a Sleeping state
		 *
		 * @param	deltaTime the elapsed time since the last update of the
		 *			PhysicsEngine in seconds */
		void checkSleepyRigidBodies(float deltaTime);
	};

}

#endif		// PHYSICS_SYSTEM_H
