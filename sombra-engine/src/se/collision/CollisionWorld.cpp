#include <algorithm>
#include "se/collision/CollisionWorld.h"
#include "se/collision/Collider.h"
#include "se/collision/ConvexCollider.h"
#include "se/collision/ConcaveCollider.h"

namespace se::collision {

	static constexpr int kMaxRayCasterIterations = 32;


	CollisionWorld::CollisionWorld(float minFDifference, float contactPrecision, float contactSeparation) :
		mFineCollisionDetector(minFDifference, contactPrecision, contactSeparation),
		mRayCaster(contactPrecision, kMaxRayCasterIterations) {}


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
		for (auto it = mCollidersManifoldMap.begin(); it != mCollidersManifoldMap.end();) {
			if (!it->second.state[Manifold::State::Intersecting]) {
				it = mCollidersManifoldMap.erase(it);
			}
			else {
				// Set the remaining Manifolds' state to not intersecting
				it->second.state.reset(Manifold::State::Intersecting);
				it->second.state.set(Manifold::State::Updated);
				++it;
			}
		}

		// Broad collision phase
		for (Collider* collider : mColliders) {
			mCoarseCollisionDetector.submit(collider);
		}
		mCoarseCollisionDetector.processIntersectingColliders([this](const ColliderPair& pair) {
			// Find a Manifold between the given colliders
			auto itPairManifold = mCollidersManifoldMap.find(std::make_pair(pair.first, pair.second));
			if (itPairManifold == mCollidersManifoldMap.end()) {
				itPairManifold = mCollidersManifoldMap.find(std::make_pair(pair.second, pair.first));
			}

			// Narrow collision phase
			if (itPairManifold != mCollidersManifoldMap.end()) {
				// Set the Manifold back to its old state (if we are at this
				// stage it was Intersecting)
				itPairManifold->second.state.set(Manifold::State::Intersecting);
				itPairManifold->second.state.reset(Manifold::State::Updated);

				// Update the Manifold data
				mFineCollisionDetector.collide(itPairManifold->second);
			}
			else {
				// Create a new Manifold
				Manifold manifold(pair.first, pair.second);
				if (mFineCollisionDetector.collide(manifold)) {
					mCollidersManifoldMap.emplace(
						std::piecewise_construct,
						std::forward_as_tuple(pair.first, pair.second),
						std::forward_as_tuple(manifold)
					);
				}
			}
		});

		// Reset the updated state of all the Colliders
		for (Collider* collider : mColliders) {
			collider->resetUpdatedState();
		}
	}


	void CollisionWorld::processCollisionManifolds(const ManifoldCallback& callback) const
	{
		for (auto it = mCollidersManifoldMap.begin(); it != mCollidersManifoldMap.end(); ++it) {
			callback(it->second);
		}
	}


	void CollisionWorld::processRayCast(
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		const RayCastCallback& callback
	) const
	{
		for (const Collider* collider : mColliders) {
			if (auto convexCollider = dynamic_cast<const ConvexCollider*>(collider)) {
				auto [intersects, rayCast] = mRayCaster.calculateRayCast(rayOrigin, rayDirection, *convexCollider);
				if (intersects) {
					callback(*collider, rayCast);
				}
			}
			/*else if (auto concaveCollider = dynamic_cast<const ConcaveCollider*>(collider)) {

			}*/
		}
	}

}
