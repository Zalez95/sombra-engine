#ifndef FRICTION_CONSTRAINT_H
#define FRICTION_CONSTRAINT_H

#include <glm/glm.hpp>
#include "Constraint.h"

namespace se::physics {

	/**
	 * Class FrictionConstraint, it's used to prevent the motion of the
	 * RigidBodies in the given Contact normal's tangent direction
	 */
	class FrictionConstraint : public Constraint
	{
	private:	// Attributes
		/** The ConstraintBounds of the FrictionConstraint */
		ConstraintBounds mConstraintBounds;

		/** The Gravity acceleration */
		float mGravityAcceleration;

		/** Both the dynamic and static friction coefficient (they are the
		 * same) */
		float mFrictionCoefficient;

		/** The vectors in world space that points from the RigidBodies center
		 * of masses to their respective contact points */
		std::array<glm::vec3, 2> mConstraintVectors;

		/** One of the tangent vector to the contact normal */
		glm::vec3 mTangent;

		/** If the FrictionConstraint has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new FrictionConstraint */
		FrictionConstraint() :
			mConstraintBounds{ 0.0f, 0.0f },
			mGravityAcceleration(0.0f), mFrictionCoefficient(0.0f),
			mConstraintVectors{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mTangent(0.0f), mUpdated(true) {};

		/** Creates a new FrictionConstraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the
		 *			Constraint
		 * @param	gravityAcceleration the gravity acceleration value
		 * @param	frictionCoefficient both the dynamic and static friction
		 *			coefficient
		 * @note	initially the constraint points are located in the RigidBody
		 *			origins */
		FrictionConstraint(
			const std::array<RigidBody*, 2>& rigidBodies,
			float gravityAcceleration, float frictionCoefficient
		) : Constraint(rigidBodies),
			mConstraintBounds{ 0.0f, 0.0f },
			mGravityAcceleration(gravityAcceleration),
			mFrictionCoefficient(frictionCoefficient),
			mConstraintVectors{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mTangent(0.0f) {};

		/** @copydoc Constraint::clone() */
		virtual std::unique_ptr<Constraint> clone() const override
		{ return std::make_unique<FrictionConstraint>(*this); };

		/** @return	the ConstraintBounds of the Constraint */
		virtual const ConstraintBounds& getConstraintBounds() const override
		{ return mConstraintBounds; };

		/** @return	the value of the Bias of the constraint */
		virtual float getBias() const override;

		/** @return	the Jacobian matrix of the constraint */
		virtual std::array<float, 12> getJacobianMatrix() const override;

		/** @copydoc Constraint::updated() */
		bool updated() const override { return mUpdated; };

		/** @copydoc Constraint::resetUpdatedState() */
		void resetUpdatedState() override { mUpdated = false; };

		/** Sets the constraint vectors of the NormalConstraint
		 *
		 * @param	constraintVectors the vectors in world space that points
		 *			from the RigidBodies center of masses to their respective
		 *			contact points */
		void setConstraintVectors(
			const std::array<glm::vec3, 2>& constraintVectors
		);

		/** Sets the tangent vector of the FrictionConstraint
		 *
		 * @param	tangent one of the tangent vectors to the contact normal */
		void setTangent(const glm::vec3& tangent);

		/** Updates the value of the constraint bounds with the given contact
		 * mass
		 *
		 * @param	contactMass the mass at the current contact point */
		void calculateConstraintBounds(float contactMass);
	};

}

#endif		// FRICTION_CONSTRAINT_H
