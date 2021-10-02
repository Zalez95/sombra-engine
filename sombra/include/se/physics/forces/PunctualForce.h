#ifndef PUNCTUAL_FORCE_H
#define PUNCTUAL_FORCE_H

#include "Force.h"

namespace se::physics {

	/**
	 * Class PunctualForce, it handles a force applied to a point of a
	 * RigidBody
	 */
	class PunctualForce : public Force
	{
	private:	// Attributes
		/** Holds the value of the Force to apply */
		glm::vec3 mValue;

		/** The point in world coordinates where the force will be applied */
		glm::vec3 mPoint;

	public:		// Functions
		/** Creates a new PunctualForce
		 *
		 * @param	value the Value of the Force to be applied
		 * @param	point the world coordinates of the point where the Force
		 *			will be applied */
		PunctualForce(
			const glm::vec3& value = glm::vec3(0.0f),
			const glm::vec3& point = glm::vec3(0.0f)
		);

		/** @return	the force value */
		const glm::vec3& getValue() const { return mValue; };

		/** Sets the force value
		 *
		 * @param	value the force value */
		void setValue(const glm::vec3& value) { mValue = value; };

		/** @return	the point in world coordinates */
		const glm::vec3& getPoint() const { return mPoint; };

		/** Sets the force point
		 *
		 * @param	point the Point in world coordinates */
		void setPoint(const glm::vec3& point) { mPoint = point; };

		/** @copydoc Force::clone() */
		virtual std::unique_ptr<Force> clone() const override
		{ return std::make_unique<PunctualForce>(*this); };

		/** @copydoc Force::calculate(const RigidBody&) */
		ForceTorquePair calculate(const RigidBody& rigidBody) const override;
	};

}

#endif		// PUNCTUAL_FORCE_H
