#include "fe/physics/ForceManager.h"
#include "fe/physics/Force.h"
#include "fe/physics/RigidBody.h"

namespace fe { namespace physics {

	void ForceManager::addRigidBody(RigidBody* rigidBody, Force* force)
	{
		mRBForces.emplace_back(rigidBody, force);
	}


	void ForceManager::removeRigidBody(RigidBody* rigidBody, Force* force)
	{
		for (auto it = mRBForces.begin(); it != mRBForces.end(); ++it) {
			if ((it->rigidBody == rigidBody) && (it->force == force)) {
				mRBForces.erase(it);
				break;
			}
		}
	}


	void ForceManager::applyForces()
	{
		// Clean the older forces
		for (auto it = mRBForces.begin(); it != mRBForces.end(); ++it) {
			it->rigidBody->forceSum	= glm::vec3(0.0f);
			it->rigidBody->torqueSum	= glm::vec3(0.0f);
		}

		// Apply the current forces
		for (auto it = mRBForces.begin(); it != mRBForces.end(); ++it) {
			it->force->apply(it->rigidBody);
		}
	}

}}
