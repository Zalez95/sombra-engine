#ifndef GRAVITY_H
#define GRAVITY_H

#include <glm/glm.hpp>
#include "Force.h"

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
		 * @param	gravity the acceleration of the gravity (m/s2) */
		Gravity(const glm::vec3& gravity = glm::vec3(0.0f, 9.807f, 0.0f));

		/** @return	the gravity acceleration value (m/s2) */
		const glm::vec3& getValue() const { return mGravity; };

		/** Sets the gravity value
		 *
		 * @param	value the new gravity acceleration value (m/s2) */
		void setValue(const glm::vec3& value) { mGravity = value; };

		/** @return	always true because the Gravity is a constant Force */
		virtual bool isConstant() const;

		/** Applies the gravitational force to the given RigidBody based in the
		 * time
		 *
		 * @param	rigidBody a pointer to the RigidBody to which we want to
		 *			apply the Force */
		void apply(RigidBody* rigidBody);
	};

}

#endif		// GRAVITY_H
