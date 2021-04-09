#include <cassert>
#include <algorithm>
#include "se/physics/RigidBody.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	RigidBodyConfig::RigidBodyConfig(float mass, const glm::mat3& inertiaTensor)
	{
		assert(mass > 0.0f && "The mass must be larger than zero");

		invertedMass			= 1.0f / mass;
		invertedInertiaTensor	= glm::inverse(inertiaTensor);
	}


	RigidBody::RigidBody(const RigidBodyConfig& config, const RigidBodyData& data) :
		mConfig(config), mData(data),
		mTransformsMatrix(1.0f),
		mInvertedInertiaTensorWorld(config.invertedInertiaTensor),
		mMotion(0.0f)
	{
		synchWithData();
	}


	RigidBody& RigidBody::addForce(ForceSPtr force)
	{
		mForces.emplace_back(std::move(force));
		RigidBodyDynamics::setState(*this, RigidBodyState::Sleeping, false);
		return *this;
	}


	RigidBody& RigidBody::removeForce(ForceSPtr force)
	{
		mForces.erase(std::remove(mForces.begin(), mForces.end(), force), mForces.end());
		RigidBodyDynamics::setState(*this, RigidBodyState::Sleeping, false);
		return *this;
	}


	void RigidBody::synchWithData()
	{
		RigidBodyDynamics::updateTransformsMatrix(*this);
		RigidBodyDynamics::setState(*this, RigidBodyState::Sleeping, false);
	}

}
