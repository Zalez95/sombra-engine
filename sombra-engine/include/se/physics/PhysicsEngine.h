#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <set>
#include <map>
#include "ForceManager.h"
#include "ConstraintManager.h"

namespace se::physics {

	struct RigidBody;


	/**
	 * Class PhysicsEngine, it's used to update the position and orientation
	 * of all the RigidBodies
	 */
	class PhysicsEngine
	{
	private:	// Attributes
		/** The ForceManager of the PhysicsEngine. It's used to store the
		 * relationships between the RigidBodies and the Forces and
		 * applying them */
		ForceManager mForceManager;

		/** The ConstraintManager of the PhysicsEngine. We will delegate all
		 * the constraint resolution to it */
		ConstraintManager mConstraintManager;

		/** All the RigidBodies that must be updated */
		std::set<RigidBody*> mRigidBodies;

	public:		// Functions
		/** Creates a new PhysicsEngine */
		PhysicsEngine() {};

		/** Class destructor */
		~PhysicsEngine() {};

		/** @return	the ForceManager of the PhysicsEngine */
		inline ForceManager& getForceManager() { return mForceManager; };

		/** @return	the ConstraintManager of the PhysicsEngine */
		inline ConstraintManager& getConstraintManager() 
		{ return mConstraintManager; };

		/** Adds the given RigidBody to the PhysicsEngine so it will
		 * be updated
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want
		 *			to update */
		void addRigidBody(RigidBody* rigidBody);

		/** Removes the given RigidBody from the PhysicsEngine so it
		 * won't longer be updated
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want
		 *			to update
		 * @note	you must call this function before deleting the
		 *			RigidBody */
		void removeRigidBody(RigidBody* rigidBody);

		/** Updates the positions of the RigidBodies added to the System
		 *
		 * @param	delta the elapsed time since the last update of the
		 *			PhysicsEngine in seconds */
		void integrate(float delta);

		/** Updates the ConstraintManager to solve the Constraints between
		 * the RigidBodies
		 *
		 * @param	delta the elapsed time since the last update of the
		 *			PhysicsEngine in seconds */
		void solveConstraints(float delta);
	};

}

#endif		// PHYSICS_SYSTEM_H
