#ifndef NORMAL_CONSTRAINT_H
#define NORMAL_CONSTRAINT_H

#include <glm/glm.hpp>
#include "Constraint.h"

namespace se::physics {

	/**
	 * Class NormalConstraint, its used to push the given RigidBodies appart
	 * from each other
	 */
	class NormalConstraint : public Constraint
	{
	private:	// Attributes
		/** The ConstraintBounds of all the NormalConstraints */
		static constexpr ConstraintBounds kConstraintBounds = {
			0.0f,
			std::numeric_limits<float>::max()
		};

		/** Controls how fast the constraint will be solved */
		float mBeta;

		/** The proportion of the closing velocity with which the RigidBodies
		 * will bounce. With a value of 0 the closing velocity will be
		 * cancelled, with a value of 1 the objects will bounce with the same
		 * velocity as they collide */
		float mRestitutionFactor;

		/** The minimum penetration value that the solver will allow before
		 * applying the NormalConstraint. It's used to reduce jitters */
		float mSlopPenetration;

		/** The minimum closing velocity value that the solver will allow before
		 * applying the NormalConstraint. It's used to reduce jitters */
		float mSlopRestitution;

		/** The vectors in world space that points from the RigidBodies center
		 * of masses to their respective contact points */
		std::array<glm::vec3, 2> mConstraintVectors;

		/** The normal vector from the first RigidBody to the second one in
		 * world space. It's an unit-length vector with an origin in the second
		 * constraint point pointing outside the first RigidBody */
		glm::vec3 mNormal;

		/** The elapsed time since the last update */
		float mDeltaTime;

	public:		// Functions
		/** Creates a new NormalConstraint */
		NormalConstraint() :
			mBeta(0.0f), mRestitutionFactor(0.0f),
			mSlopPenetration(0.0f), mSlopRestitution(0.0f),
			mConstraintVectors{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mNormal(0.0f), mDeltaTime(0.0f) {};

		/** Creates a new NormalConstraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the Constraint
		 * @param	beta controls how fast the constraint will be solved
		 * @param	restitutionFactor the proportion of the closing velocity
		 *			with which the RigidBodies will bounce
		 * @param	slopPenetration the minimum penetration value allowed before
		 *			applying the NormalConstraint
		 * @param	slopRestitution the minimum closing velocity value allowed
		 *			before applying the NormalConstraint
		 * @note	initially the constraint points are located in the RigidBody
		 *			origins */
		NormalConstraint(
			const std::array<RigidBody*, 2>& rigidBodies,
			float beta, float restitutionFactor,
			float slopPenetration, float slopRestitution
		) : Constraint(rigidBodies),
			mBeta(beta), mRestitutionFactor(restitutionFactor),
			mSlopPenetration(slopPenetration),
			mSlopRestitution(slopRestitution),
			mConstraintVectors{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mNormal(0.0f), mDeltaTime(0.0f) {};

		/** Class destructor */
		virtual ~NormalConstraint() = default;

		/** @return	the ConstraintBounds of the Constraint */
		virtual const ConstraintBounds& getConstraintBounds() const override
		{ return kConstraintBounds; };

		/** @return	the value of the Bias of the constraint */
		virtual float getBias() const override;

		/** @return	the Jacobian matrix of the constraint */
		virtual std::array<float, 12> getJacobianMatrix() const override;

		/** Sets the constraint vectors of the NormalConstraint
		 *
		 * @param	constraintVectors the vectors in world space that points
		 *			from the RigidBodies center of masses to their respective
		 *			contact points */
		void setConstraintVectors(
			const std::array<glm::vec3, 2>& constraintVectors
		) { mConstraintVectors = constraintVectors; };

		/** Sets the normal vector of the NormalConstraint
		 *
		 * @param	normal the new normal vector */
		void setNormal(const glm::vec3& normal) { mNormal = normal; };

		/** Sets the elapsed time since the last update of the NormalConstraint
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void setDeltaTime(float deltaTime) { mDeltaTime = deltaTime; };
	};

}

#endif		// NORMAL_CONSTRAINT_H
