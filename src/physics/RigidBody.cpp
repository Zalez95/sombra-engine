#include "RigidBody.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

namespace physics {

	RigidBody::RigidBody(
		float mass, float linearSlowDown,
		const glm::mat3& inertiaTensor, float angularSlowDown
	) {
		assert(mass > 0);

		mInvertedMass			= 1.0f / mass;
		mLinearSlowDown			= linearSlowDown;	// TODO: SlowDown to [0-1]?
		mInvertedInertiaTensor	= glm::inverse(inertiaTensor);
		mAngularSlowDown		= angularSlowDown;	// TODO: SlowDown to [0-1]?

		updateInertiaTensorWorld();
	}


	void RigidBody::addForce(const glm::vec3& force)
	{
		mForceSum	+= force;
	}


	void RigidBody::addForceAtPoint(const glm::vec3& force, const glm::vec3& point)
	{
		mForceSum	+= force;
		mTorqueSum	+= point * force;
	}


	void RigidBody::addForceAtLocalPoint(const glm::vec3& force, const glm::vec3& point)
	{
		glm::vec3 pointWorld = glm::mat3(mTransformsMatrix) * point + glm::vec3(mTransformsMatrix[3]);
		addForceAtPoint(force, pointWorld);
	}


	void RigidBody::cleanForces()
	{
		mForceSum	= glm::vec3();
		mTorqueSum	= glm::vec3();
	}


	void RigidBody::integrate(float delta)
	{
		// Update the Position
		mLinearAcceleration = mInvertedMass * mForceSum;
		mLinearVelocity *= glm::pow(mLinearSlowDown, delta);
		mLinearVelocity += mLinearAcceleration * delta;
		mPosition += mLinearVelocity * delta;

		// Update the Orientation
		mAngularAcceleration = mInvertedInertiaTensorWorld * mTorqueSum;
		mAngularVelocity *= glm::pow(mAngularSlowDown, delta);
		mAngularVelocity += mAngularAcceleration * delta;
		mOrientation *= glm::quat(mAngularVelocity) * delta;
		mOrientation = glm::normalize(mOrientation);

		// Update the derived data
		updateTransformsMatrix();
		updateInertiaTensorWorld();
	}


	void RigidBody::updateTransformsMatrix()
	{
		glm::mat4 translation	= glm::translate(glm::mat4(), mPosition);
		glm::mat4 rotation		= glm::mat4_cast(mOrientation);
		mTransformsMatrix = translation * rotation;
	}


	void RigidBody::updateInertiaTensorWorld()
	{
		glm::mat3 transformsMat3(mTransformsMatrix);

		mInvertedInertiaTensorWorld = glm::transpose(inverse(transformsMat3))
			* mInvertedInertiaTensor
			* glm::inverse(transformsMat3);
	}

}
