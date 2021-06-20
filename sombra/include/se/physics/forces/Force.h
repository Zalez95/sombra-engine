#ifndef FORCE_H
#define FORCE_H

#include <memory>

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

		/** @return	a pointer to a copy of the current Force */
		virtual std::unique_ptr<Force> clone() const = 0;

		/** Applyes the force to the given RigidBody
		 *
		 * @param	rigidBody the RigidBody to which we want to apply the
		 *			Force */
		virtual void apply(RigidBody& rigidBody) = 0;
	};

}

#endif		// FORCE_H
