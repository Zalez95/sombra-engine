#include <algorithm>
#include "se/physics/RigidBody.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	void RigidBodyDynamics::processForces(RigidBody& rigidBody)
	{
		rigidBody.getData().forceSum = glm::vec3(0.0f);
		rigidBody.getData().torqueSum = glm::vec3(0.0f);
		rigidBody.processForces([&](std::shared_ptr<Force> force) {
			force->apply(rigidBody);
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
		rigidBody.mData.linearAcceleration = rigidBody.mConfig.invertedMass * rigidBody.mData.forceSum;
	}


	void RigidBodyDynamics::applyTorques(RigidBody& rigidBody)
	{
		rigidBody.mData.angularAcceleration = rigidBody.mInvertedInertiaTensorWorld * rigidBody.mData.torqueSum;
	}


	void RigidBodyDynamics::integrateLinearAcceleration(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.mData.linearVelocity = rigidBody.mData.linearVelocity * glm::pow(rigidBody.mConfig.linearDrag, deltaTime)
			+ rigidBody.mData.linearAcceleration * deltaTime;
	}


	void RigidBodyDynamics::integrateAngularAcceleration(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.mData.angularVelocity = rigidBody.mData.angularVelocity * glm::pow(rigidBody.mConfig.angularDrag, deltaTime)
			+ rigidBody.mData.angularAcceleration * deltaTime;
	}


	void RigidBodyDynamics::integrateLinearVelocity(RigidBody& rigidBody, float deltaTime)
	{
		rigidBody.mData.position += rigidBody.mData.linearVelocity * deltaTime;
	}


	void RigidBodyDynamics::integrateAngularVelocity(RigidBody& rigidBody, float deltaTime)
	{
		const glm::quat angularVelocityQuat(0.0f, rigidBody.mData.angularVelocity);
		rigidBody.mData.orientation += (0.5f * deltaTime * angularVelocityQuat) * rigidBody.mData.orientation;
		rigidBody.mData.orientation = glm::normalize(rigidBody.mData.orientation);
	}


	void RigidBodyDynamics::updateTransformsMatrix(RigidBody& rigidBody)
	{
		// Update the transforms matrix of the RigidBody
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), rigidBody.mData.position);
		glm::mat4 rotation		= glm::mat4_cast(rigidBody.mData.orientation);
		rigidBody.mTransformsMatrix = translation * rotation;

		// Update the inertia tensor of the RigidBody
		glm::mat3 inverseTransformsMat3 = glm::inverse(rigidBody.mTransformsMatrix);
		rigidBody.mInvertedInertiaTensorWorld = glm::transpose(inverseTransformsMat3)
			* rigidBody.mConfig.invertedInertiaTensor
			* inverseTransformsMat3;
	}


	void RigidBodyDynamics::updateMotion(RigidBody& rigidBody, float bias, float maxMotion)
	{
		float motion = glm::dot(rigidBody.mData.linearVelocity, rigidBody.mData.linearVelocity)
			+ glm::dot(rigidBody.mData.angularVelocity, rigidBody.mData.angularVelocity);
		motion = bias * rigidBody.mMotion + (1.0f - bias) * motion;
		rigidBody.mMotion = std::min(motion, maxMotion);
	}


	void RigidBodyDynamics::setState(RigidBody& rigidBody, RigidBodyState state, bool value)
	{
		if (value) {
			rigidBody.mState.set( static_cast<int>(state) );

			// Set the motion of the RigidBody to a larger value than
			// mSleepEpsilon to prevent it from falling sleep instantly
			if (state == RigidBodyState::Sleeping) {
				rigidBody.mMotion = 2.0f * rigidBody.mConfig.sleepMotion;
			}
		}
		else {
			rigidBody.mState.reset( static_cast<int>(state) );

			if (state == RigidBodyState::Sleeping) {
				rigidBody.mMotion = 0.0f;
			}
		}
	}

}
