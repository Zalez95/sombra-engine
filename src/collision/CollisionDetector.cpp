#include "CollisionDetector.h"
#include <algorithm>
#include <set>

namespace collision {

	void CollisionDetector::addCollider(const Collider* collider)
	{
		if (collider) {
			mColliders.push_back(collider);
		}
	}


	void CollisionDetector::removeCollider(const Collider* collider)
	{
		mColliders.erase(
			std::remove(mColliders.begin(), mColliders.end(), collider),
			mColliders.end()
		);
	}


	void CollisionDetector::update()
	{
		// Broad collision phase
		for (const Collider* collider : mColliders) {
			mCoarseCollisionDetector.submit(collider);
		}
		auto intersectingColliders = mCoarseCollisionDetector.getIntersectingColliders();

		// Narrow collision phase
		mManifolds.clear();
		std::set<ColliderPair> collidingManifolds;
		for (ColliderPair pair : intersectingColliders) {
			const Collider* c1 = pair.first;
			const Collider* c2 = pair.second;

			// Create a manifold between the colliders if it doesn't exist
			if (mMapCollidersManifolds.find(pair) == mMapCollidersManifolds.end()) {
				mMapCollidersManifolds.emplace(
					std::piecewise_construct,
					std::forward_as_tuple(pair),
					std::forward_as_tuple(c1, c2)
				);
			}

			Manifold* manifold = &mMapCollidersManifolds.at(pair);
			if (mFineCollisionDetector.collide(*c1, *c2, *manifold)) {
				collidingManifolds.emplace(pair);
				mManifolds.push_back(manifold);
			}
		}

		// Delete the old contact manifolds between non intersecting colliders
		for (auto it = mMapCollidersManifolds.begin(); it != mMapCollidersManifolds.end();) {
			if (collidingManifolds.find(it->first) == collidingManifolds.end()) {
				it = mMapCollidersManifolds.erase(it);
			}
			else {
				++it;
			}
		}
	}

}
