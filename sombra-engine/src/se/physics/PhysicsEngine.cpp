#include <algorithm>
#include "se/physics/PhysicsEngine.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	void PhysicsEngine::addRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mRigidBodies.push_back(rigidBody);
	}


	void PhysicsEngine::removeRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mRigidBodies.erase(
			std::remove(mRigidBodies.begin(), mRigidBodies.end(), rigidBody),
			mRigidBodies.end()
		);
	}


	void PhysicsEngine::integrate(float delta)
	{
		// Reset the RigidBodies Updated state
		for (RigidBody* rigidBody : mRigidBodies) {
			RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Integrated, false);
		}

		// Apply all the forces
		mForceManager.applyForces();

		float bias = std::pow(mBaseBias, delta);
		for (RigidBody* rigidBody : mRigidBodies) {
			if (!rigidBody->checkState(RigidBodyState::Sleeping)) {
				// Simulate the RigidBody motion
				RigidBodyDynamics::integrate(*rigidBody, delta);
				RigidBodyDynamics::updateTransformsMatrix(*rigidBody);
				RigidBodyDynamics::updateMotion(*rigidBody, bias);
				RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Integrated, true);

				if (rigidBody->getMotion() < rigidBody->getConfig().sleepMotion) {
					// Put the RigidBody to Sleeping state
					RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Sleeping, true);
				}
			}
		}
	}


	void PhysicsEngine::solveConstraints(float delta)
	{
		// Reset the RigidBodies Updated state
		for (RigidBody* rigidBody : mRigidBodies) {
			RigidBodyDynamics::setState(*rigidBody, RigidBodyState::ConstraintsSolved, false);
		}

		mConstraintManager.update(delta);
	}

}
