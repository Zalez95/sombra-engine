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


	void updateTransforms(RigidBody& rigidBody)
	{
		// Update the transforms matrix of the RigidBody
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), rigidBody.position);
		glm::mat4 rotation		= glm::mat4_cast(rigidBody.orientation);
		rigidBody.transformsMatrix = translation * rotation;

		// Update the inertia tensor of the RigidBody
		glm::mat3 inverseTransformsMat3 = glm::inverse(rigidBody.transformsMatrix);
		rigidBody.invertedInertiaTensorWorld = glm::transpose(inverseTransformsMat3)
			* rigidBody.invertedInertiaTensor
			* inverseTransformsMat3;
	}


	void updateMotion(RigidBody& rigidBody, float bias)
	{
		float motion = glm::dot(rigidBody.linearVelocity, rigidBody.linearVelocity)
			+ glm::dot(rigidBody.angularVelocity, rigidBody.angularVelocity);
		rigidBody.motion = bias * rigidBody.motion + (1.0f - bias) * motion;
	}

}
