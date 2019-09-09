#ifndef FORCE_MANAGER_H
#define FORCE_MANAGER_H

#include <vector>

namespace se::physics {

	class RigidBody;
	class Force;


	/**
	 * Class ForceManager
	 */
	class ForceManager
	{
	private:	// Nested types
		/** Struct RBForce, it's used to pair RigidBodies with Forces
		 * in the ForceManager */
		struct RBForce
		{
			/** A pointer to the RigidBody that we want to pair */
			RigidBody* rigidBody;

			/** A pointer to the force that we want to pair */
			Force* force;

			/** Creates a new RBForce
			 *
			 * @param	rigidBody a pointer to the RigidBody of RBForce
			 * @param	force a pointer to the Force of RBForce */
			RBForce(RigidBody* rigidBody, Force* force) :
				rigidBody(rigidBody), force(force) {};
		};

	private:	// Attributes
		/** The vector that pairs the RigidBodies with the Forces */
		std::vector<RBForce> mRBForces;

	public:		// Forces
		/** Subscribes the given RigidBody to the given Force, so the
		 * RigidBody will be affected by the Force
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to be
		 *			affected by the Force
		 * @param	force a pointer to the Force that will affect the
		 *			RigidBody */
		void addRBForce(RigidBody* rigidBody, Force* force);

		/** Unsubscribes the given RigidBody from given Force so it won't longer
		 * be affected by the Force
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to
		 *			unsubscribe
		 * @param	force a pointer to the Force that we want to unsubscribe */
		void removeRBForce(RigidBody* rigidBody, Force* force);

		/** Unsubscribes the given RigidBody from all its Forces so it won't
		 * longer be affected by them
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to
		 *			unsubscribe */
		void removeRigidBody(RigidBody* rigidBody);

		/** Removes the given Force from the ForceManager so it will no longer
		 * affect its subscribers
		 *
		 * @param	force a pointer to the Force that we want to unsubscribe */
		void removeForce(Force* force);

		/** Applies the Forces stored in the ForceManager to their RigidBody
		 * subscribers
		 * @note	the Forces will be applied only to non Sleeping
		 *			RigidBodies */
		void applyForces();
	};

}

#endif		// FORCE_MANAGER_H
