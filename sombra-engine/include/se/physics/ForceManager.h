#ifndef FORCE_MANAGER_H
#define FORCE_MANAGER_H

#include <vector>

namespace se::physics {

	struct RigidBody;
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
		/** Registers the given RigidBody with the given Force, so the
		 * RigidBody will be affected by the Force
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to be
		 *			affected by the Force
		 * @param	force a pointer to the Force that we want to register */
		void addRigidBody(RigidBody* rigidBody, Force* force);

		/** Unregisters the given RigidBody and given Force so it won't longer
		 * be affected by the Force
		 *
		 * @param	rigidBody a pointer to the RigidBody that we want to
		 *			unregister
		 * @param	force a pointer to the Force that we want to unregister */
		void removeRigidBody(RigidBody* rigidBody, Force* force);

		/** Applies the forces stored in the ForceManager */
		void applyForces();
	};

}

#endif		// FORCE_MANAGER_H
