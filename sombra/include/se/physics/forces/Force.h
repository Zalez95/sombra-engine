#ifndef FORCE_H
#define FORCE_H

namespace se::physics {

	class RigidBody;


	/**
	 * Class Force, it represents a force that can change the movement of the
	 * Particles and the RigidBodies
	 */
	class Force
	{
	public:		// Functions
		/** Class destructor */
		virtual ~Force() = default;

		/** @return	true if the Force has a constant value that will always be
		 *			applied to the RigidBodies through all its existence (like
		 *			gravity), false otherwise */
		virtual bool isConstant() const = 0;

		/** Applyes the force to the given RigidBody
		 *
		 * @param	rigidBody a pointer to the RigidBody to which we want to
		 *			apply the Force */
		virtual void apply(RigidBody* rigidBody) = 0;
	};

}

#endif		// FORCE_H
