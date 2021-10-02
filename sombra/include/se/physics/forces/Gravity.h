#ifndef GRAVITY_H
#define GRAVITY_H

#include "Force.h"

namespace se::physics {

	/**
	 * Class Gravity, it handles a gravitational force
	 */
	class Gravity : public Force
	{
	private:	// Attributes
		/** Holds the acceleration of the gravity */
		float mGravity;

	public:		// Functions
		/** Creates a new Gravity Force
		 *
		 * @param	gravity the acceleration of the gravity (m/s2) */
		Gravity(const float& gravity = 9.807f);

		/** @return	the gravity acceleration value (m/s2) */
		const float& getValue() const { return mGravity; };

		/** Sets the gravity value
		 *
		 * @param	value the new gravity acceleration value (m/s2) */
		void setValue(const float& value) { mGravity = value; };

		/** @copydoc Force::clone() */
		virtual std::unique_ptr<Force> clone() const override
		{ return std::make_unique<Gravity>(*this); };

		/** @copydoc Force::calculate(const RigidBody&) */
		ForceTorquePair calculate(const RigidBody& rigidBody) const override;
	};

}

#endif		// GRAVITY_H
