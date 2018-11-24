#include <set>
#include <algorithm>
#include "se/collision/CollisionDetector.h"

namespace se::collision {

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
			Manifold* manifold = getManifold(pair.first, pair.second);
			if (mFineCollisionDetector.collide(*manifold)) {
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

// Private functions
	Manifold* CollisionDetector::getManifold(const Collider* c1, const Collider* c2)
	{
		Manifold* ret;

		auto it = mMapCollidersManifolds.find(std::make_pair(c1, c2));
		if (it != mMapCollidersManifolds.end()) {
			ret = &it->second;
		}
		else {
			it = mMapCollidersManifolds.find(std::make_pair(c2, c1));
			if (it != mMapCollidersManifolds.end()) {
				ret = &it->second;
			}
			else {
				it = mMapCollidersManifolds.emplace(
					std::piecewise_construct,
					std::forward_as_tuple(c1, c2),
					std::forward_as_tuple(c1, c2)
				).first;
				ret = &it->second;
			}
		}

		return ret;
	}

}