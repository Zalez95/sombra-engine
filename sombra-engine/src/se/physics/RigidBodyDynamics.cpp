#include "se/physics/RigidBodyDynamics.h"
#include "se/physics/RigidBody.h"

namespace se::physics {

	void integrate(RigidBody& rigidBody, float deltaTime)
	{
		// Update the linear attributes
		applyForces(rigidBody);
		integrateLinearAcceleration(rigidBody, deltaTime);
		integrateLinearVelocity(rigidBody, deltaTime);

		// Update the angular attributes
		applyTorques(rigidBody);
		integrateAngularAcceleration(rigidBody, deltaTime);
		integrateAngularVelocity(rigidBody, deltaTime);

		updateRigidBodyData(rigidBody);
	}


	void applyForces(RigidBody& rigidBody)
	{
		rigidBody.linearAcceleration = rigidBody.invertedMass * rigidBody.forceSum;
	}


	void applyTorques(RigidBody& rigidBody)
	{
		rigidBody.angularAcceleration = rigidBody.invertedInertiaTensorWorld * rigidBody.torqueSum;
	}


	void integrateLinearAcceleration(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.linearVelocity = rigidBody.linearVelocity * glm::pow(rigidBody.linearSlowDown, deltaTime)
			+ rigidBody.linearAcceleration * deltaTime;
	}


	void integrateAngularAcceleration(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.angularVelocity = rigidBody.angularVelocity * glm::pow(rigidBody.angularSlowDown, deltaTime)
			+ rigidBody.angularAcceleration * deltaTime;
	}


	void integrateLinearVelocity(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.position += rigidBody.linearVelocity * deltaTime;
	}


	void integrateAngularVelocity(RigidBody& rigidBody, float deltaTime)
	{
		const glm::quat angularVelocityQuat(0.0f, rigidBody.angularVelocity);
		rigidBody.orientation += (0.5f * deltaTime * angularVelocityQuat) * rigidBody.orientation;
		rigidBody.orientation = glm::normalize(rigidBody.orientation);
	}

}
