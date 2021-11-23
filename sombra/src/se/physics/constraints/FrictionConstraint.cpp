#include "se/physics/constraints/FrictionConstraint.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	float FrictionConstraint::getBias() const
	{
		return 0.0f;
	}


	std::array<float, 12> FrictionConstraint::getJacobianMatrix() const
	{
		glm::vec3 r1xn = glm::cross(mConstraintVectors[0], mTangent);
		glm::vec3 r2xn = glm::cross(mConstraintVectors[1], mTangent);

		return {
			-mTangent.x, -mTangent.y, -mTangent.z,
			-r1xn.x, -r1xn.y, -r1xn.z,
			mTangent.x, mTangent.y, mTangent.z,
			r2xn.x, r2xn.y, r2xn.z
		};
	}


	void FrictionConstraint::setConstraintVectors(const std::array<glm::vec3, 2>& constraintVectors)
	{
		mConstraintVectors = constraintVectors;
		mUpdated = true;
	}


	void FrictionConstraint::setTangent(const glm::vec3& tangent)
	{
		mTangent = tangent;
		mUpdated = true;
	}


	void FrictionConstraint::calculateConstraintBounds(float contactMass)
	{
		mConstraintBounds.lambdaMax = mFrictionCoefficient * contactMass * mGravityAcceleration;
		mConstraintBounds.lambdaMin = -mConstraintBounds.lambdaMax;
		mUpdated = true;
	}

}
