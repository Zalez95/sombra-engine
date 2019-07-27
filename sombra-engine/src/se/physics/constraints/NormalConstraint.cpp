#include "se/physics/constraints/NormalConstraint.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	constexpr ConstraintBounds NormalConstraint::kConstraintBounds;


	float NormalConstraint::getBias() const
	{
		glm::vec3 p1 = mRigidBodies[0]->position + mConstraintVectors[0];
		glm::vec3 p2 = mRigidBodies[1]->position + mConstraintVectors[1];
		float penetration = glm::dot(p2 - p1, mNormal);
		float biasError = 0.0f;
		if (std::abs(penetration) > mSlopPenetration) {
			biasError = -mBeta * penetration / mDeltaTime;
		}

		glm::vec3 v1 = mRigidBodies[0]->linearVelocity
			+ glm::cross(mRigidBodies[0]->angularVelocity, mConstraintVectors[0]);
		glm::vec3 v2 = mRigidBodies[1]->linearVelocity
			+ glm::cross(mRigidBodies[1]->angularVelocity, mConstraintVectors[1]);
		float closingVelocity = glm::dot(v2 - v1, mNormal);
		float biasRestitution = 0.0f;
		if (std::abs(closingVelocity) > mSlopRestitution) {
			biasRestitution = mRestitutionFactor * closingVelocity;
		}

		return biasError + biasRestitution;
	}


	std::array<float, 12> NormalConstraint::getJacobianMatrix() const
	{
		glm::vec3 r1xn = glm::cross(mConstraintVectors[0], mNormal);
		glm::vec3 r2xn = glm::cross(mConstraintVectors[1], mNormal);

		return {
			-mNormal.x, -mNormal.y, -mNormal.z,
			-r1xn.x, -r1xn.y, -r1xn.z,
			mNormal.x, mNormal.y, mNormal.z,
			r2xn.x, r2xn.y, r2xn.z
		};
	}

}
