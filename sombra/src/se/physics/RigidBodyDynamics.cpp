#include "se/physics/RigidBody.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	void RigidBodyDynamics::processForces(RigidBody& rigidBody)
	{
		rigidBody.mState.forceSum = glm::vec3(0.0f);
		rigidBody.mState.torqueSum = glm::vec3(0.0f);
		rigidBody.processForces([&](const std::shared_ptr<Force>& force) {
			auto [curForce, curTorque] = force->calculate(rigidBody);
			rigidBody.mState.forceSum += curForce;
			rigidBody.mState.torqueSum += curTorque;
		});
	}


	void RigidBodyDynamics::integrate(RigidBody& rigidBody, float deltaTime)
	{
		// Update the linear attributes
		applyForces(rigidBody);
		integrateLinearAcceleration(rigidBody, deltaTime);
		integrateLinearVelocity(rigidBody, deltaTime);

		// Update the angular attributes
		applyTorques(rigidBody);
		integrateAngularAcceleration(rigidBody, deltaTime);
		integrateAngularVelocity(rigidBody, deltaTime);
	}


	void RigidBodyDynamics::applyForces(RigidBody& rigidBody)
	{
		rigidBody.mState.linearAcceleration = rigidBody.mProperties.invertedMass * rigidBody.mState.forceSum;
	}


	void RigidBodyDynamics::applyTorques(RigidBody& rigidBody)
	{
		rigidBody.mState.angularAcceleration = rigidBody.mState.invertedInertiaTensorWorld * rigidBody.mState.torqueSum;
	}


	void RigidBodyDynamics::integrateLinearAcceleration(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.mState.linearVelocity = rigidBody.mState.linearVelocity * glm::pow(1.0f - rigidBody.mProperties.linearDrag, deltaTime)
			+ rigidBody.mState.linearAcceleration * deltaTime;
	}


	void RigidBodyDynamics::integrateAngularAcceleration(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.mState.angularVelocity = rigidBody.mState.angularVelocity * glm::pow(1.0f - rigidBody.mProperties.angularDrag, deltaTime)
			+ rigidBody.mState.angularAcceleration * deltaTime;
	}


	void RigidBodyDynamics::integrateLinearVelocity(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.mState.position += rigidBody.mState.linearVelocity * deltaTime;
	}


	void RigidBodyDynamics::integrateAngularVelocity(RigidBody& rigidBody, float deltaTime)
	{
		const glm::quat angularVelocityQuat(0.0f, rigidBody.mState.angularVelocity);
		rigidBody.mState.orientation += (0.5f * deltaTime * angularVelocityQuat) * rigidBody.mState.orientation;
		rigidBody.mState.orientation = glm::normalize(rigidBody.mState.orientation);
	}

}
