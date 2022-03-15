#include "se/physics/RigidBodyWorld.h"
#include "se/physics/constraints/Constraint.h"
#include "se/physics/constraints/ConstraintManager.h"

namespace se::physics {

	void ConstraintManager::addConstraint(Constraint* constraint)
	{
		std::scoped_lock lck(mMutex);

		bool rbHasIsland[2] = { false, false };
		std::size_t iRBIsland[2] = { 0, 0 };

		for (std::size_t i = 0; i < 2; ++i) {
			RigidBody* rb = constraint->getRigidBody(i);
			if (rb->getProperties().type != RigidBodyProperties::Type::Static) {
				auto it = std::find_if(mIslands.begin(), mIslands.end(), [rb](const ConstraintIsland& island) {
					return island.hasRigidBody(rb);
				});
				if (it != mIslands.end()) {
					rbHasIsland[i] = true;
					iRBIsland[i] = std::distance(mIslands.begin(), it);
				}
			}
		}

		std::size_t iConstraintIsland = 0;
		if (rbHasIsland[0]) {
			if (rbHasIsland[1]) {	// Merge islands
				if (iRBIsland[0] < iRBIsland[1]) {
					iConstraintIsland = iRBIsland[0];
					mIslands[iRBIsland[0]].merge(mIslands[iRBIsland[1]]);
					mIslands.erase(mIslands.begin() + iRBIsland[1]);
				}
				else if (iRBIsland[0] > iRBIsland[1]) {
					iConstraintIsland = iRBIsland[1];
					mIslands[iRBIsland[1]].merge(mIslands[iRBIsland[0]]);
					mIslands.erase(mIslands.begin() + iRBIsland[0]);
				}
				else {
					iConstraintIsland = iRBIsland[0];
				}
			}
			else {					// 0
				iConstraintIsland = iRBIsland[0];
			}
		}
		else {
			if (rbHasIsland[1]) {	// 1
				iConstraintIsland = iRBIsland[1];
			}
			else {					// New island
				mIslands.emplace_back(mParentWorld->getProperties().maxConstraintIterations);
				iConstraintIsland = mIslands.size() - 1;
			}
		}

		mIslands[iConstraintIsland].addConstraint(constraint);
	}


	bool ConstraintManager::hasConstraints() const
	{
		std::scoped_lock lck(mMutex);
		return !mIslands.empty();
	}


	void ConstraintManager::removeConstraint(Constraint* constraint)
	{
		std::scoped_lock lck(mMutex);
		for (ConstraintIsland& island : mIslands) {
			if (island.removeConstraint(constraint)) {
				if (!island.hasConstraints()) {
					std::swap(island, mIslands.back());
					mIslands.pop_back();
				}
				break;
			}
		}
	}


	void ConstraintManager::removeRigidBody(RigidBody* rigidBody)
	{
		std::scoped_lock lck(mMutex);
		for (auto it = mIslands.begin(); it != mIslands.end();) {
			if (it->removeRigidBody(rigidBody) && !it->hasConstraints()) {
				it = mIslands.erase(it);
			}
			else {
				++it;
			}
		}
	}


	void ConstraintManager::update(float deltaTime)
	{
		std::vector<Constraint*> updatedConstraints;

		{ // Get the Constraints of the RigidBodies whose properties have changed
			std::scoped_lock lck(mMutex);
			for (ConstraintIsland& island : mIslands) {
				island.processRigidBodies([&](RigidBody* rb) {
					if (rb->getStatus(RigidBody::Status::PropertiesChanged)) {
						island.processRigidBodyConstraints(rb, [&](Constraint* constraint) {
							updatedConstraints.push_back(constraint);
						});
					}
				});
			}
		}

		// Remove and add those Constraints so the RigidBodies whose properties
		// have changed from static to dynamic are in the same island
		for (Constraint* constraint : updatedConstraints) {
			removeConstraint(constraint);
			addConstraint(constraint);
		}

		{ // Solve the islands constraints
			std::scoped_lock lck(mMutex);

			std::size_t nThreads = mParentWorld->getProperties().numThreads;
			std::size_t islandsPerThread = mIslands.size() / nThreads;
			std::vector<std::future<void>> threadFutures(nThreads);

			for (std::size_t iThread = 0; iThread < nThreads; ++iThread) {
				threadFutures[iThread] = mParentWorld->getThreadPool().async([=]() {
					std::size_t iStart = iThread * islandsPerThread;
					std::size_t iEnd = (iThread < nThreads - 1)?
						(iThread + 1) * islandsPerThread :
						mIslands.size();

					for (std::size_t i = iStart; i < iEnd; ++i) {
						mIslands[i].update(deltaTime);
					}
				});
			}

			for (auto& future : threadFutures) {
				future.get();
			}
		}
	}

}
