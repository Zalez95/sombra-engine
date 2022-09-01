#include <algorithm>
#include "se/physics/LogWrapper.h"
#include "se/physics/RigidBodyWorld.h"
#include "RigidBodyDynamics.h"

namespace se::physics {

	LogHandler WorldProperties::sDefaultLogHandler = LogHandler();


	RigidBodyWorld::RigidBodyWorld(const WorldProperties& properties) :
		mProperties(properties), mThreadPool(properties.numThreads),
		mCollisionDetector(*this), mConstraintManager(*this), mCollisionSolver(*this)
	{
		mCollisionDetector.addListener(&mCollisionSolver);
	}


	RigidBodyWorld::~RigidBodyWorld()
	{
		mCollisionDetector.removeListener(&mCollisionSolver);
	}


	void RigidBodyWorld::addRigidBody(RigidBody* rigidBody)
	{
		SPHYS_DEBUG_LOG(*this) << "Adding RigidBody " << rigidBody;

		if (!rigidBody) { return; }

		std::scoped_lock lck(mMutex);

		auto it = std::lower_bound(mRigidBodies.begin(), mRigidBodies.end(), rigidBody);
		std::size_t iRB = std::distance(mRigidBodies.begin(), it);
		if ((it != mRigidBodies.end()) && ((*it) == rigidBody)) { return; }

		mRigidBodies.insert(it, rigidBody);
		mRigidBodiesColliders.insert(mRigidBodiesColliders.begin() + iRB, rigidBody->getCollider());
		if (mRigidBodiesColliders[iRB]) {
			mCollisionDetector.addCollider(mRigidBodiesColliders[iRB]);
		}

		SPHYS_DEBUG_LOG(*this) << "Added RigidBody " << rigidBody;
	}


	void RigidBodyWorld::removeRigidBody(RigidBody* rigidBody)
	{
		SPHYS_DEBUG_LOG(*this) << "Removing RigidBody " << rigidBody;

		std::scoped_lock lck(mMutex);

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

		SPHYS_DEBUG_LOG(*this) << "Removed RigidBody " << rigidBody;
	}


	void RigidBodyWorld::update(float deltaTime)
	{
		std::scoped_lock lck(mMutex);

		// Update the RigidBodies based on the user input
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

		// Detect collisions
		mCollisionDetector.update();

		// Substeps update
		float substepTime = deltaTime / mProperties.numSubsteps;
		for (std::size_t substep = 0; substep < mProperties.numSubsteps; ++substep) {
			// Simulate the RigidBody dynamics
			for (RigidBody* rigidBody : mRigidBodies) {
				if ((rigidBody->getProperties().type == RigidBodyProperties::Type::Dynamic)
					&& !rigidBody->getStatus(RigidBody::Status::Sleeping)
				) {
					if (isInside(mProperties.worldAABB, rigidBody->getState().position, mProperties.collisionProperties.coarseEpsilon)) {
						RigidBodyDynamics::processForces(*rigidBody);
						RigidBodyDynamics::integrate(*rigidBody, substepTime);
					}
					else {
						SPHYS_DEBUG_LOG(*this) << "RigidBody " << rigidBody << " outside world bounds";
					}
				}
			}

			// Update the collision solver
			mCollisionSolver.update(substepTime);

			// Solve the RigidBody Constraints
			mConstraintManager.update(substepTime);
		}

		// Update the RigidBodies status and transforms
		float bias = std::pow(mProperties.motionBias, deltaTime);
		for (RigidBody* rigidBody : mRigidBodies) {
			if (rigidBody->getProperties().type == RigidBodyProperties::Type::Static) {
				rigidBody->setStatus(RigidBody::Status::Sleeping, true);

				if (rigidBody->getStatus(RigidBody::Status::StateChanged)) {
					rigidBody->updateTransforms();
				}
			}
			else if (!rigidBody->getStatus(RigidBody::Status::Sleeping)) {
				rigidBody->updateTransforms();
				rigidBody->updateMotion(bias, 10.0f * rigidBody->getProperties().sleepMotion);

				if (rigidBody->getState().motion < rigidBody->getProperties().sleepMotion) {
					SPHYS_DEBUG_LOG(*this) << "RigidBody " << rigidBody << " is going to sleep";
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
