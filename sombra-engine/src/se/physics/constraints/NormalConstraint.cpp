#include "se/physics/constraints/NormalConstraint.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	constexpr ConstraintBounds NormalConstraint::kConstraintBounds;


	float NormalConstraint::getBias() const
	{
		glm::vec3 p1 = mRigidBodies[0]->position + mConstraintVectors[0];
		glm::vec3 p2 = mRigidBodies[1]->position + mConstraintVectors[1];
		float penetration = glm::dot(p2 - p1, mNormal);

		// If the penetration is large enough we try to separate the RigidBodies
		float biasError = 0.0f;
		if (std::abs(penetration) > mSlopPenetration) {
			biasError = -mBeta * penetration / mDeltaTime;
		}

		glm::vec3 v1 = mRigidBodies[0]->linearVelocity
			+ glm::cross(mRigidBodies[0]->angularVelocity, mConstraintVectors[0]);
		glm::vec3 v2 = mRigidBodies[1]->linearVelocity
			+ glm::cross(mRigidBodies[1]->angularVelocity, mConstraintVectors[1]);
		float closingVelocity = glm::dot(v2 - v1, mNormal);

		// If the closing velocity is large enough we try to remove it
		float biasRestitution = 0.0f;
		if (std::abs(closingVelocity) > mSlopRestitution) {
			// Remove the velocity built up from the acceleration in the
			// previous frame
			glm::vec3 vFromAcceleration1 = mRigidBodies[0]->linearAcceleration * mDeltaTime;
			glm::vec3 vFromAcceleration2 = mRigidBodies[1]->linearAcceleration * mDeltaTime;
			float vFromAcceleration = glm::dot(vFromAcceleration2 - vFromAcceleration1, mNormal);

			biasRestitution = vFromAcceleration + mRestitutionFactor * (closingVelocity - vFromAcceleration);
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
