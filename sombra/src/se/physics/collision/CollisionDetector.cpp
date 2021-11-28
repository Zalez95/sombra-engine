#include <algorithm>
#include "se/utils/Log.h"
#include "se/physics/RigidBodyWorld.h"
#include "se/physics/collision/CollisionDetector.h"

namespace se::physics {

	CollisionDetector::CollisionDetector(RigidBodyWorld& parentWorld) :
		mParentWorld(parentWorld),
		mCoarseCollisionDetector(mParentWorld.getProperties().coarseCollisionEpsilon),
		mFineCollisionDetector(
			mParentWorld.getProperties().coarseCollisionEpsilon,
			mParentWorld.getProperties().minFDifference, mParentWorld.getProperties().maxIterations,
			mParentWorld.getProperties().contactPrecision, mParentWorld.getProperties().contactSeparation,
			mParentWorld.getProperties().raycastPrecision
		)
	{
		mManifolds.reserve(mParentWorld.getProperties().maxCollidingRBs);
		mCollidersManifoldMap.reserve(mParentWorld.getProperties().maxCollidingRBs);
	}


	void CollisionDetector::addCollider(Collider* collider)
	{
		mCoarseCollisionDetector.add(collider);
	}


	void CollisionDetector::removeCollider(Collider* collider)
	{
		mCoarseCollisionDetector.remove(collider);

		for (auto it = mCollidersManifoldMap.begin(); it != mCollidersManifoldMap.end();) {
			if ((collider == it->first.first) || (collider == it->first.second)) {
				mManifolds.erase(mManifolds.begin().setIndex(it->second));
				it = mCollidersManifoldMap.erase(it);
			}
			else {
				++it;
			}
		}
	}


	void CollisionDetector::update()
	{
		// Clean old non intersecting Manifolds
		for (auto it = mCollidersManifoldMap.begin(); it != mCollidersManifoldMap.end();) {
			if (!mManifolds[it->second].state[Manifold::State::Intersecting]) {
				mManifolds.erase( mManifolds.begin().setIndex(it->second) );
				it = mCollidersManifoldMap.erase(it);
			}
			else {
				// Set the remaining Manifolds' state to not intersecting
				mManifolds[it->second].state.reset(Manifold::State::Intersecting);
				mManifolds[it->second].state.set(Manifold::State::Updated);
				++it;
			}
		}

		// Broad collision phase
		mCoarseCollisionDetector.update();
		mCoarseCollisionDetector.calculateCollisions([this](Collider* collider1, Collider* collider2) {
			// Skip non updated Colliders and Colliders without any common layer
			if ((collider1->updated() || collider2->updated())
				&& ((collider1->getLayers() & collider2->getLayers()).any())
			) {
				// Find a Manifold between the given colliders
				ColliderPair sortedPair = (collider1 <= collider2)? ColliderPair(collider1, collider2)
										: ColliderPair(collider2, collider1);
				auto itPairManifold = mCollidersManifoldMap.find(sortedPair);

				// Narrow collision phase
				if (itPairManifold != mCollidersManifoldMap.end()) {
					// Set the Manifold back to its old state (if we are at this
					// stage it was Intersecting)
					mManifolds[itPairManifold->second].state.set(Manifold::State::Intersecting);
					mManifolds[itPairManifold->second].state.reset(Manifold::State::Updated);

					// Update the Manifold data
					mFineCollisionDetector.collide(mManifolds[itPairManifold->second]);
				}
				else {
					// Create a new Manifold
					if (mManifolds.size() < mManifolds.capacity()) {
						Manifold manifold(sortedPair.first, sortedPair.second);
						if (mFineCollisionDetector.collide(manifold)) {
							auto manifoldIndex = mManifolds.emplace(std::move(manifold)).getIndex();
							mCollidersManifoldMap.emplace(
								std::piecewise_construct,
								std::forward_as_tuple(sortedPair.first, sortedPair.second),
								std::forward_as_tuple(manifoldIndex)
							);
						}
					}
					else {
						SOMBRA_ERROR_LOG << "Can't create more Manifolds";
					}
				}
			}
		});

		// Reset the updated state of all the Colliders
		mCoarseCollisionDetector.processColliders([](Collider* collider) {
			collider->resetUpdatedState();
		});

		// Notify the ICollisionListeners
		for (ICollisionListener* listener : mListeners) {
			for (auto& manifold : mManifolds) {
				listener->onCollision(manifold);
			}
		}
	}


	void CollisionDetector::addListener(ICollisionListener* listener)
	{
		if (listener) {
			mListeners.push_back(listener);
		}
	}


	void CollisionDetector::removeListener(ICollisionListener* listener)
	{
		mListeners.erase(
			std::remove(mListeners.begin(), mListeners.end(), listener),
			mListeners.end()
		);
	}


	void CollisionDetector::rayCastAll(const Ray& ray, const RayCastCallback& callback)
	{
		mCoarseCollisionDetector.calculateIntersections(ray, [&](Collider* collider) {
			auto [intersects, rayHit] = mFineCollisionDetector.intersects(ray, *collider);
			if (intersects) {
				callback(collider, rayHit);
			}
		});
	}


	std::pair<Collider*, RayHit> CollisionDetector::rayCastFirst(const Ray& ray)
	{
		Collider* collider = nullptr;
		RayHit rayHit;
		rayHit.distance = std::numeric_limits<float>::max();

		mCoarseCollisionDetector.calculateIntersections(ray, [&](Collider* collider2) {
			auto [intersects, rayHit2] = mFineCollisionDetector.intersects(ray, *collider2);
			if (intersects && (!collider || (rayHit2.distance < rayHit.distance))) {
				collider = collider2;
				rayHit = rayHit2;
			}
		});

		return { collider, rayHit };
	}

}
