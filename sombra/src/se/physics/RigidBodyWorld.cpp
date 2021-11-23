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

		auto it = std::lower_bound(mRigidBodies.begin(), mRigidBodies.end(), rigidBody);
		std::size_t iRB = std::distance(mRigidBodies.begin(), it);
		if ((it != mRigidBodies.end()) && ((*it) == rigidBody)) { return; }

		mRigidBodies.insert(it, rigidBody);
		mRigidBodiesColliders.insert(mRigidBodiesColliders.begin() + iRB, rigidBody->getCollider());
		if (mRigidBodiesColliders[iRB]) {
			mCollisionDetector.addCollider(mRigidBodiesColliders[iRB]);
		}
	}


	void RigidBodyWorld::removeRigidBody(RigidBody* rigidBody)
	{
		auto it = std::lower_bound(mRigidBodies.begin(), mRigidBodies.end(), rigidBody);
		std::size_t iRB = std::distance(mRigidBodies.begin(), it);
		if ((it == mRigidBodies.end()) || ((*it) != rigidBody)) { return; }

		mCollisionSolver.removeRigidBody(rigidBody);
		mConstraintManager.removeRigidBody(rigidBody);
		if (mRigidBodiesColliders[iRB]) {
			mCollisionDetector.removeCollider(mRigidBodiesColliders[iRB]);
		}

		mRigidBodiesColliders.erase(mRigidBodiesColliders.begin() + iRB);
		mRigidBodies.erase(it);
	}


	void RigidBodyWorld::update(float deltaTime)
	{
		float bias = std::pow(mProperties.motionBias, deltaTime);

		// Update the RigidBodies status
		for (auto it = mRigidBodies.begin(); it != mRigidBodies.end(); ++it) {
			if ((*it)->getStatus(RigidBody::Status::StateChanged)
				|| (*it)->getStatus(RigidBody::Status::ForcesChanged)
			) {
				// Wake up the Dynamic RigidBodies updated by the user
				bool isStatic = ((*it)->getProperties().type == RigidBodyProperties::Type::Static);
				(*it)->setStatus(RigidBody::Status::Sleeping, isStatic);
			}

			if ((*it)->getStatus(RigidBody::Status::ColliderChanged)) {
				// Change the Collider in the CollisionDetector
				std::size_t iRB = std::distance(mRigidBodies.begin(), it);

				mCollisionDetector.removeCollider(mRigidBodiesColliders[iRB]);
				mRigidBodiesColliders[iRB] = (*it)->getCollider();
				if (mRigidBodiesColliders[iRB]) {
					mCollisionDetector.addCollider(mRigidBodiesColliders[iRB]);
				}
			}
		}

		// Simulate the RigidBody dynamics
		for (RigidBody* rigidBody : mRigidBodies) {
			if ((rigidBody->getProperties().type == RigidBodyProperties::Type::Dynamic)
				&& isInside(mProperties.worldAABB, rigidBody->getState().position, mProperties.coarseCollisionEpsilon)
				&& !rigidBody->getStatus(RigidBody::Status::Sleeping)
			) {
				RigidBodyDynamics::processForces(*rigidBody);
				RigidBodyDynamics::integrate(*rigidBody, deltaTime);
				rigidBody->updateTransforms();
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
			if (rigidBody->getProperties().type == RigidBodyProperties::Type::Static) {
				rigidBody->setStatus(RigidBody::Status::Sleeping, true);
			}
			else if (!rigidBody->getStatus(RigidBody::Status::Sleeping)) {
				rigidBody->updateMotion(bias, 10.0f * rigidBody->getProperties().sleepMotion);

				if (rigidBody->getState().motion < rigidBody->getProperties().sleepMotion) {
					rigidBody->setStatus(RigidBody::Status::Sleeping, true);
				}
			}

			rigidBody->setStatus(RigidBody::Status::PropertiesChanged, false);
			rigidBody->setStatus(RigidBody::Status::StateChanged, false);
			rigidBody->setStatus(RigidBody::Status::ColliderChanged, false);
			rigidBody->setStatus(RigidBody::Status::ForcesChanged, false);
		}
	}

}
