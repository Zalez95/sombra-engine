#include "se/physics/constraints/NormalConstraint.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	float NormalConstraint::getBias() const
	{
		// The penetration and restitution slops are based on
		// http://www.allenchou.net/2014/01/game-physics-stability-slops/

		float biasError = 0.0f, biasRestitution = 0.0f;

		// If the penetration is large enough we try to separate the RigidBodies
		glm::vec3 p1 = mRigidBodies[0]->getState().position + mConstraintVectors[0];
		glm::vec3 p2 = mRigidBodies[1]->getState().position + mConstraintVectors[1];
		float penetration = glm::dot(p1 - p2, mNormal);
		if (penetration > mSlopPenetration) {
			biasError = mBeta * (penetration - mSlopPenetration) / mDeltaTime;

			// If the closing velocity is large enough we try to remove it
			glm::vec3 v1 = mRigidBodies[0]->getState().linearVelocity
				+ glm::cross(mRigidBodies[0]->getState().angularVelocity, mConstraintVectors[0]);
			glm::vec3 v2 = mRigidBodies[1]->getState().linearVelocity
				+ glm::cross(mRigidBodies[1]->getState().angularVelocity, mConstraintVectors[1]);
			float closingVelocity = glm::dot(v1 - v2, mNormal);
			if (closingVelocity > mSlopRestitution) {
				biasRestitution = mRestitutionFactor * (closingVelocity - mSlopRestitution);
			}
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


	void NormalConstraint::setConstraintVectors(const std::array<glm::vec3, 2>& constraintVectors)
	{
		mConstraintVectors = constraintVectors;
		mUpdated = true;
	}


	void NormalConstraint::setNormal(const glm::vec3& normal)
	{
		mNormal = normal;
		mUpdated = true;
	}


	void NormalConstraint::setDeltaTime(float deltaTime)
	{
		mDeltaTime = deltaTime;
		mUpdated = true;
	}

}
