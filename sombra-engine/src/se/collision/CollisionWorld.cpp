#include <algorithm>
#include "se/collision/CollisionWorld.h"
#include "se/collision/Collider.h"

namespace se::collision {

	CollisionWorld::CollisionWorld() :
		mFineCollisionDetector(kMinFDifference, kContactPrecision, kContactSeparation) {}


	void CollisionWorld::addCollider(Collider* collider)
	{
		if (!collider) { return; }

		mColliders.push_back(collider);
	}


	void CollisionWorld::removeCollider(Collider* collider)
	{
		mColliders.erase(
			std::remove(mColliders.begin(), mColliders.end(), collider),
			mColliders.end()
		);
	}


	void CollisionWorld::update()
	{
		// Clean old non intersecting Manifolds
		for (std::size_t i = 0; i < mManifolds.size();) {
			Manifold* pManifold = const_cast<Manifold*>(mManifolds[i]);

			if (pManifold->state == ManifoldState::Disjoint) {
				auto itPair = mMapCollidersManifolds.find(std::make_pair(pManifold->colliders[0], pManifold->colliders[1]));
				if (itPair != mMapCollidersManifolds.end()) {
					mMapCollidersManifolds.erase(itPair);
				}

				if (i + 1 < mManifolds.size()) {
					std::swap(mManifolds[i], mManifolds.back());
				}
				mManifolds.pop_back();
			}
			else {
				// Set the remaining Manifolds' state to Disjoint
				pManifold->state = ManifoldState::Disjoint;
				pManifold->updated = true;
				++i;
			}
		}

		// Broad collision phase
		for (Collider* collider : mColliders) {
			mCoarseCollisionDetector.submit(collider);
		}
		auto intersectingColliders = mCoarseCollisionDetector.getIntersectingColliders();

		// Narrow collision phase
		for (ColliderPair& pair : intersectingColliders) {
			// Find a Manifold between the given colliders
			auto itPairManifold = mMapCollidersManifolds.find(std::make_pair(pair.first, pair.second));
			if (itPairManifold == mMapCollidersManifolds.end()) {
				itPairManifold = mMapCollidersManifolds.find(std::make_pair(pair.second, pair.first));
			}

			if (itPairManifold != mMapCollidersManifolds.end()) {
				// Set the Manifold back to its old state (if we are at this
				// stage it was Intersecting)
				itPairManifold->second.state = ManifoldState::Intersecting;
				itPairManifold->second.updated = false;

				// Update the Manifold data
				mFineCollisionDetector.collide(itPairManifold->second);
			}
			else {
				// Create a new Manifold
				Manifold manifold(pair.first, pair.second, ManifoldState::Disjoint);
				if (mFineCollisionDetector.collide(manifold)) {
					itPairManifold = mMapCollidersManifolds.emplace(
						std::piecewise_construct,
						std::forward_as_tuple(pair.first, pair.second),
						std::forward_as_tuple(manifold)
					).first;
					mManifolds.push_back(&itPairManifold->second);
				}
			}
		}

		// Reset the updated state of all the Colliders
		for (Collider* collider : mColliders) {
			collider->resetUpdatedState();
		}
	}

}
