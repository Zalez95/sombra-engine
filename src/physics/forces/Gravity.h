#ifndef GRAVITY_H
#define GRAVITY_H

#include <glm/glm.hpp>
#include "Force.h"
#include "../Particle.h"
#include "../RigidBody.h"

namespace physics {

	/**
	 * Class Gravity, it handles a gravitational force
	 */
	class Gravity : public Force
	{
	private:	// Attributes
		/** Holds the acceleration of the gravity */
		glm::vec3 mGravity;

	public:		// Functions
		/** Creates a new Gravity Force
		 * 
		 * @param	gravity the acceleration of the gravity */
		Gravity(const glm::vec3& gravity) : mGravity(gravity) {};

		/** Class destructor */
		~Gravity() {};

		/** Applyes the gravitational force to the given particle
		 *
		 * @param	particle a pointer to the Particle to which we want to
		 *			apply the Force */
		void apply(Particle* particle)
		{
			particle->addForce(mGravity * particle->getMass());
		};
		
		/** Applyes the gravitational force to the given RigidBody based in the
		 * time
		 *
		 * @param	rigidBody a pointer to the RigidBody to which we want to
		 *			apply the Force */
		void apply(RigidBody* rigidBody)
		{
			if (rigidBody->hasFiniteMass()) {
				rigidBody->addForce(mGravity * rigidBody->getMass());
			}
		};
	};

}

#endif		// GRAVITY_H
