#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "se/physics/RigidBody.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	RigidBodyConfig::RigidBodyConfig() :
		invertedMass(0.0f), invertedInertiaTensor(0.0f),
		linearDrag(0.0f), angularDrag(0.0f),
		frictionCoefficient(0.0f), sleepMotion(0.001f) {}


	RigidBodyConfig::RigidBodyConfig(float mass, const glm::mat3& inertiaTensor) :
		linearDrag(0.0f), angularDrag(0.0f),
		frictionCoefficient(0.0f), sleepMotion(0.001f)
	{
		assert(mass > 0.0f && "The mass must be larger than zero");

		invertedMass			= 1.0f / mass;
		invertedInertiaTensor	= glm::inverse(inertiaTensor);
	}


	RigidBodyData::RigidBodyData() :
		position(0.0f), orientation(1.0f, glm::vec3(0.0f)),
		linearVelocity(0.0), angularVelocity(0.0),
		linearAcceleration(0.0f), angularAcceleration(0.0f),
		forceSum(0.0f), torqueSum(0.0f) {}


	RigidBody::RigidBody(const RigidBodyConfig& config, const RigidBodyData& data) :
		mConfig(config), mData(data),
		mTransformsMatrix(1.0f),
		mInvertedInertiaTensorWorld(config.invertedInertiaTensor),
		mMotion(0.0f)
	{
		synchWithData();
	}


	void RigidBody::synchWithData()
	{
		RigidBodyDynamics::updateTransformsMatrix(*this);
		RigidBodyDynamics::setState(*this, RigidBodyState::Sleeping, false);
	}

}
