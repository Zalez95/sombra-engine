#ifndef FORCE_H
#define FORCE_H

#include <memory>
#include <glm/glm.hpp>

namespace se::physics {

	class RigidBody;


	/**
	 * Class Force, it represents a force that can change the movement of the
	 * Particles and the RigidBodies
	 */
	class Force
	{
	public:		// Nested types
		using ForceTorquePair = std::pair<glm::vec3, glm::vec3>;

	public:		// Functions
		/** Class destructor */
		virtual ~Force() = default;

		/** @return	a pointer to a copy of the current Force */
		virtual std::unique_ptr<Force> clone() const = 0;

		/** Calculates the force and torque to apply to the given RigidBody
		 *
		 * @param	rigidBody the RigidBody to which we want to apply the
		 *			Force
		 * @return	a pair with the Force and Torque to apply */
		virtual ForceTorquePair calculate(const RigidBody& rigidBody) const = 0;
	};

}

#endif		// FORCE_H
