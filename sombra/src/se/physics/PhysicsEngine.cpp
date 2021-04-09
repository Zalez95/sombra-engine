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

		mConstraintManager.removeRigidBody(rigidBody);

		mRigidBodies.erase(
			std::remove(mRigidBodies.begin(), mRigidBodies.end(), rigidBody),
			mRigidBodies.end()
		);
	}


	void PhysicsEngine::resetRigidBodiesState()
	{
		for (RigidBody* rigidBody : mRigidBodies) {
			RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Integrated, false);
			RigidBodyDynamics::setState(*rigidBody, RigidBodyState::ConstraintsSolved, false);
		}
	}


	void PhysicsEngine::integrate(float deltaTime)
	{
		for (RigidBody* rigidBody : mRigidBodies) {
			if (glm::all(glm::greaterThan(rigidBody->getData().position, mMinWorldAABB))
				&& glm::all(glm::lessThan(rigidBody->getData().position, mMaxWorldAABB))
				&& !rigidBody->checkState(RigidBodyState::Sleeping)
			) {
				RigidBodyDynamics::processForces(*rigidBody);
				RigidBodyDynamics::integrate(*rigidBody, deltaTime);
				RigidBodyDynamics::updateTransformsMatrix(*rigidBody);
				RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Integrated, true);
			}
		}
	}


	void PhysicsEngine::solveConstraints(float deltaTime)
	{
		mConstraintManager.update(deltaTime);
	}


	void PhysicsEngine::checkSleepyRigidBodies(float deltaTime)
	{
		float bias = std::pow(mBaseBias, deltaTime);

		for (RigidBody* rigidBody : mRigidBodies) {
			if (!rigidBody->checkState(RigidBodyState::Sleeping)) {
				RigidBodyDynamics::updateMotion(*rigidBody, bias, 10.0f * rigidBody->getConfig().sleepMotion);

				if (rigidBody->getMotion() < rigidBody->getConfig().sleepMotion) {
					// Put the RigidBody to Sleeping state
					RigidBodyDynamics::setState(*rigidBody, RigidBodyState::Sleeping, true);
				}
			}
		}
	}

}
