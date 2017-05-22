#ifndef FORCE_H
#define FORCE_H

namespace physics {

	class Particle;
	class RigidBody;


	/**
	 * Class Force, it represents a force that can change the movement of the
	 * Particles and the RigidBodies
	 */
	class Force
	{
	public:		// Functions
		/** Applyes the force to the given Particle
		 * 
		 * @param	particle a pointer to the Particle to which we want to
		 *			apply the Force */
		virtual void apply(Particle* particle) = 0;

		/** Applyes the force to the given RigidBody
		 * 
		 * @param	rigidBody a pointer to the RigidBody to which we want to
		 *			apply the Force */
		virtual void apply(RigidBody* rigidBody) = 0;
	};

}

#endif		// FORCE_H
