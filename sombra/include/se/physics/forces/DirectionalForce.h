#ifndef DIRECTIONAL_FORCE_H
#define DIRECTIONAL_FORCE_H

#include "Force.h"

namespace se::physics {

	/**
	 * Class DirectionalForce, it handles a force applied to the center of mass
	 * of a RigidBody
	 */
	class DirectionalForce : public Force
	{
	private:	// Attributes
		/** Holds the value of the Force to apply */
		glm::vec3 mValue;

	public:		// Functions
		/** Creates a new DirectionalForce
		 *
		 * @param	value the Value of the Force to be applied */
		DirectionalForce(const glm::vec3& value = glm::vec3(0.0f));

		/** @return	the force value */
		const glm::vec3& getValue() const { return mValue; };

		/** Sets the force value
		 *
		 * @param	value the force value */
		void setValue(const glm::vec3& value) { mValue = value; };

		/** @copydoc Force::clone() */
		virtual std::unique_ptr<Force> clone() const override
		{ return std::make_unique<DirectionalForce>(*this); };

		/** @copydoc Force::calculate(const RigidBody&) */
		ForceTorquePair calculate(const RigidBody& rigidBody) const override;
	};

}

#endif		// DIRECTIONAL_FORCE_H
