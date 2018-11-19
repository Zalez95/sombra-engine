#ifndef GRAVITY_H
#define GRAVITY_H

#include <glm/glm.hpp>
#include "../Force.h"

namespace se::physics {

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

		/** Applies the gravitational force to the given RigidBody based in the
		 * time
		 *
		 * @param	rigidBody a pointer to the RigidBody to which we want to
		 *			apply the Force */
		void apply(RigidBody* rigidBody);
	};

}

#endif		// GRAVITY_H
