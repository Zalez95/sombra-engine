#ifndef NORMAL_CONSTRAINT_H
#define NORMAL_CONSTRAINT_H

#include <glm/glm.hpp>
#include "../Constraint.h"

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

		/** The restitution factor of the NormalConstraint, it tells how much
		 * the RigidBodies will bounce after the constraint resolution. It must
		 * be a value between 0 and 1 */
		float mRestitutionFactor;

		/** The minimum penetration value that the solver will allow before
		 * applying the NormalConstraint. It's used to reduce jitters */
		float mSlopPenetration;

		/** The minimum closing velocity value that the solver will allow before
		 * applying the NormalConstraint. It's used to reduce jitters */
		float mSlopRestitution;

		/** The positions of the RigidBodies that will be affected by the
		 * constraint in local space */
		std::array<glm::vec3, 2> mConstraintPoints;

		/** The normal vector from the first RigidBody to the second one in
		 * local space */
		glm::vec3 mNormal;

		/** The elapsed time since the last update */
		float mDeltaTime;

	public:		// Functions
		/** Creates a new NormalConstraint */
		NormalConstraint() :
			mBeta(0.0f), mRestitutionFactor(0.0f),
			mSlopPenetration(0.0f), mSlopRestitution(0.0f),
			mConstraintPoints{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mNormal(0.0f), mDeltaTime(0.0f) {};

		/** Creates a new NormalConstraint
		 *
		 * @param	rigidBodies the two rigidBodies affected by the Constraint
		 * @param	beta controls how fast the constraint will be solved
		 * @param	restitutionFactor controls how much the rigidBodies will
		 *			bounce
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
		) : Constraint(&kConstraintBounds, rigidBodies),
			mBeta(beta), mRestitutionFactor(restitutionFactor),
			mSlopPenetration(slopPenetration),
			mSlopRestitution(slopRestitution),
			mConstraintPoints{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mNormal(0.0f), mDeltaTime(0.0f) {};

		/** Class destructor */
		virtual ~NormalConstraint() = default;

		/** @return the value of the Bias of the constraint */
		float getBias() const override;

		/** @return the Jacobian matrix of the constraint */
		std::array<float, 12> getJacobianMatrix() const override;

		/** Sets the constraint points of the NormalConstraint
		 *
		 * @param	constraintPoints the positions of the RigidBodies that
		 *			will be affected by the constraint in local space */
		void setConstraintPoints(
			const std::array<glm::vec3, 2>& constraintPoints
		) { mConstraintPoints = constraintPoints; };

		/** Sets the normal vector of the NormalConstraint
		 *
		 * @param	normal the normal unit-length vector at the first constraint
		 *			point pointing outside the first RigidBody */
		void setNormal(const glm::vec3& normal) { mNormal = normal; };

		/** Sets the elapsed time since the last update of the NormalConstraint
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		void setDeltaTime(float deltaTime) { mDeltaTime = deltaTime; };
	};

}

#endif		// NORMAL_CONSTRAINT_H
