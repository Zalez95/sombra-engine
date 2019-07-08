#include "se/physics/constraints/NormalConstraint.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	constexpr ConstraintBounds NormalConstraint::kConstraintBounds;


	float NormalConstraint::getBias() const
	{
		glm::vec3 p1 = mRigidBodies[0]->position + mConstraintPoints[0];
		glm::vec3 p2 = mRigidBodies[1]->position + mConstraintPoints[1];
		float penetration = glm::dot(p2 - p1, mNormal);
		float biasError = -mBeta * std::max(penetration - mSlopPenetration, 0.0f) / mDeltaTime;

		glm::vec3 v1 = mRigidBodies[0]->linearVelocity
			+ glm::cross(mRigidBodies[0]->angularVelocity, mConstraintPoints[0]);
		glm::vec3 v2 = mRigidBodies[1]->linearVelocity
			+ glm::cross(mRigidBodies[1]->angularVelocity, mConstraintPoints[1]);
		float closingVelocity = glm::dot(v2 - v1, mNormal);
		float biasRestitution = mRestitutionFactor * std::max(closingVelocity - mSlopRestitution, 0.0f);

		return biasError + biasRestitution;
	}


	std::array<float, 12> NormalConstraint::getJacobianMatrix() const
	{
		glm::vec3 r1xn = glm::cross(mConstraintPoints[0], mNormal);
		glm::vec3 r2xn = glm::cross(mConstraintPoints[1], mNormal);

		return {
			-mNormal.x, -mNormal.y, -mNormal.z,
			-r1xn.x, -r1xn.y, -r1xn.z,
			mNormal.x, mNormal.y, mNormal.z,
			r2xn.x, r2xn.y, r2xn.z
		};
	}

}
