#include <algorithm>
#include "se/physics/RigidBodyWorld.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	RigidBodyWorld::RigidBodyWorld(const WorldProperties& properties) :
		mProperties(properties), mCollisionDetector(*this),
		mConstraintManager(*this), mCollisionSolver(*this)
	{
		mCollisionDetector.addListener(&mCollisionSolver);
	}


	RigidBodyWorld::~RigidBodyWorld()
	{
		mCollisionDetector.removeListener(&mCollisionSolver);
	}


	void RigidBodyWorld::addRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mRigidBodies.push_back(rigidBody);
	}


	void RigidBodyWorld::removeRigidBody(RigidBody* rigidBody)
	{
		if (!rigidBody) { return; }

		mCollisionSolver.removeRigidBody(rigidBody);
		mConstraintManager.removeRigidBody(rigidBody);
		mCollisionDetector.removeRigidBody(rigidBody);

		mRigidBodies.erase(
			std::remove(mRigidBodies.begin(), mRigidBodies.end(), rigidBody),
			mRigidBodies.end()
		);
	}


	void RigidBodyWorld::update(float deltaTime)
	{
		float bias = std::pow(mProperties.motionBias, deltaTime);

		// Simulate the RigidBody dynamics
		for (RigidBody* rigidBody : mRigidBodies) {
			if ((rigidBody->getProperties().type == RigidBodyProperties::Type::Dynamic)
				&& glm::all(glm::greaterThan(rigidBody->getState().position, mProperties.minWorldAABB))
				&& glm::all(glm::lessThan(rigidBody->getState().position, mProperties.maxWorldAABB))
			) {
				// Wake up the RigidBodies updated by the user
				if (rigidBody->getStatus(RigidBodyState::Status::UpdatedByUser)) {
					rigidBody->setStatus(RigidBodyState::Status::UpdatedByUser, false);
					rigidBody->setStatus(RigidBodyState::Status::Sleeping, false);
				}

				if (!rigidBody->getStatus(RigidBodyState::Status::Sleeping)) {
					RigidBodyDynamics::processForces(*rigidBody);
					RigidBodyDynamics::integrate(*rigidBody, deltaTime);
					rigidBody->updateTransforms();
				}
			}
		}

		// Detect collisions
		mCollisionDetector.update();

		// Update the collision solver
		mCollisionSolver.update(deltaTime);

		// Solve the RigidBody Constraints
		mConstraintManager.update(deltaTime);

		// Put RigidBodies to sleep
		for (RigidBody* rigidBody : mRigidBodies) {
			if ((rigidBody->getProperties().type == RigidBodyProperties::Type::Dynamic)
				&& !rigidBody->getStatus(RigidBodyState::Status::Sleeping)
			) {
				rigidBody->updateMotion(bias, 10.0f * rigidBody->getProperties().sleepMotion);

				if (rigidBody->getState().motion < rigidBody->getProperties().sleepMotion) {
					rigidBody->setStatus(RigidBodyState::Status::Sleeping, true);
				}
			}
		}
	}

}
