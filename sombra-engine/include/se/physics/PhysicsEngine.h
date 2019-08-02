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

		/** The maximum motion value that a RigidBody can have to be put in a
		 * Sleep state */
		const float mSleepEpsilon;

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
		 *			motion value
		 * @param	sleepEpsilon the maximum motion value that a RigidBody can
		 *			have to be put in a Sleep state */
		PhysicsEngine(float baseBias, float sleepEpsilon) :
			mBaseBias(baseBias), mSleepEpsilon(sleepEpsilon) {};

		/** @return	the ForceManager of the PhysicsEngine */
		ForceManager& getForceManager() { return mForceManager; };

		/** @return	the ConstraintManager of the PhysicsEngine */
		ConstraintManager& getConstraintManager()
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

		/** Updates the state of the given RigidBody with the changes made to
		 * its properties
		 *
		 * @param	rigidBody a pointer to RigidBody to update
		 * @note	this function must be called each time the RigidBody's
		 *			properties are changed externally */
		void updateRigidBody(RigidBody* rigidBody);

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
	private:
		/** Sets the RigidBody's sleep state to the given one
		 *
		 * @param	rigidBody the RigidBody to update
		 * @param	value the new value of the sleep state */
		void setRigidBodySleepState(RigidBody& rigidBody, bool value) const;
	};

}

#endif		// PHYSICS_SYSTEM_H
