#include "fe/collision/CollisionDetector.h"
#include <algorithm>
#include <set>

namespace fe { namespace collision {

// Public functions
	void CollisionDetector::addCollider(const Collider* collider)
	{
		if (!collider) { return; }

		mColliders.emplace(collider);
	}


	void CollisionDetector::removeCollider(const Collider* collider)
	{
		mColliders.erase(collider);
	}


	void CollisionDetector::update()
	{
		// Broad collision phase
		for (const Collider* collider : mColliders) {
			mCoarseCollisionDetector.submit(collider);
		}
		std::set<ColliderPair> intersectingColliders = mCoarseCollisionDetector.getIntersectingColliders();

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
			if (mFineCollisionDetector.collide(c1, c2, *manifold)) {
				collidingManifolds.insert(pair);
				mManifolds.insert(manifold);
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

}}
