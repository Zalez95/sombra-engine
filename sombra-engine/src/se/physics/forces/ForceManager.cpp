#include <algorithm>
#include "se/physics/RigidBody.h"
#include "se/physics/RigidBodyDynamics.h"
#include "se/physics/forces/ForceManager.h"
#include "se/physics/forces/Force.h"

namespace se::physics {

	void ForceManager::addRBForce(RigidBody* rigidBody, Force* force)
	{
		RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Sleeping, false);

		mRBForces.emplace_back(rigidBody, force);
	}


	void ForceManager::removeRBForce(RigidBody* rigidBody, Force* force)
	{
		RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Sleeping, false);

		mRBForces.erase(
			std::remove_if(mRBForces.begin(), mRBForces.end(), [&](const RBForce& rbForce) {
				return (rbForce.rigidBody == rigidBody) && (rbForce.force == force);
			}),
			mRBForces.end()
		);
	}


	void ForceManager::removeRigidBody(RigidBody* rigidBody)
	{
		RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Sleeping, false);

		mRBForces.erase(
			std::remove_if(mRBForces.begin(), mRBForces.end(), [&](const RBForce& rbForce) {
				return (rbForce.rigidBody == rigidBody);
			}),
			mRBForces.end()
		);
	}


	void ForceManager::removeForce(Force* force)
	{
		for (auto it = mRBForces.begin(); it != mRBForces.end();) {
			if (it->force == force) {
				RigidBodyDynamics::setState(*it->rigidBody, RigidBodyState::Sleeping, false);
				it = mRBForces.erase(it);
			}
			else {
				++it;
			}
		}
	}


	void ForceManager::applyForces()
	{
		// Clean the older Forces
		for (auto& rbForce : mRBForces) {
			rbForce.rigidBody->getData().forceSum = glm::vec3(0.0f);
			rbForce.rigidBody->getData().torqueSum = glm::vec3(0.0f);
		}

		// Apply the current Forces
		for (auto& rbForce : mRBForces) {
			rbForce.force->apply(rbForce.rigidBody);
			RigidBodyDynamics::setState(*rbForce.rigidBody, RigidBodyState::Sleeping, false);
		}
	}

}
