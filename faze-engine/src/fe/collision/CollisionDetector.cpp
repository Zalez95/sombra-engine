#include "fe/collision/CollisionDetector.h"
#include <algorithm>
#include <set>
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"

namespace fe { namespace collision {

// Nested types definition
	struct CollisionDetector::Implementation
	{
		/** The CoarseCollisionDetector of the CollisionDetector. We will use
		 * it to check what Colliders are intersecting in the broad phase of
		 * the collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the CollisionDetector. We will use
		 * it to generate all the contact data */
		FineCollisionDetector mFineCollisionDetector;
	};

// Public functions
	CollisionDetector::CollisionDetector() :
		mImpl(std::make_unique<Implementation>()) {}


	CollisionDetector::~CollisionDetector() {}


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
			mImpl->mCoarseCollisionDetector.submit(collider);
		}
		auto intersectingColliders = mImpl->mCoarseCollisionDetector.getIntersectingColliders();

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
			if (mImpl->mFineCollisionDetector.collide(c1, c2, *manifold)) {
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
