#include <algorithm>
#include "se/physics/RigidBody.h"
#include "se/physics/forces/ForceManager.h"
#include "se/physics/forces/Force.h"

namespace se::physics {

	void ForceManager::addRBForce(RigidBody* rigidBody, Force* force)
	{
		mRBForces.emplace_back(rigidBody, force);
	}


	void ForceManager::removeRBForce(RigidBody* rigidBody, Force* force)
	{
		mRBForces.erase(
			std::remove_if(mRBForces.begin(), mRBForces.end(), [&](const RBForce& rbForce) {
				return (rbForce.rigidBody == rigidBody) && (rbForce.force == force);
			}),
			mRBForces.end()
		);
	}


	void ForceManager::removeRigidBody(RigidBody* rigidBody)
	{
		mRBForces.erase(
			std::remove_if(mRBForces.begin(), mRBForces.end(), [&](const RBForce& rbForce) {
				return (rbForce.rigidBody == rigidBody);
			}),
			mRBForces.end()
		);
	}


	void ForceManager::removeForce(Force* force)
	{
		mRBForces.erase(
			std::remove_if(mRBForces.begin(), mRBForces.end(), [&](const RBForce& rbForce) {
				return (rbForce.force == force);
			}),
			mRBForces.end()
		);
	}


	void ForceManager::applyForces()
	{
		// Clean the older forces
		for (auto it = mRBForces.begin(); it != mRBForces.end(); ++it) {
			it->rigidBody->forceSum = glm::vec3(0.0f);
			it->rigidBody->torqueSum = glm::vec3(0.0f);
		}

		// Apply the current forces
		for (auto it = mRBForces.begin(); it != mRBForces.end(); ++it) {
			it->force->apply(it->rigidBody);
		}
	}

}
