#include <algorithm>
#include "se/utils/Log.h"
#include "se/physics/RigidBodyWorld.h"
#include "se/physics/collision/CollisionDetector.h"

namespace se::physics {

	CollisionDetector::CollisionDetector(RigidBodyWorld& parentWorld) :
		mParentWorld(parentWorld),
		mCoarseCollisionDetector(mParentWorld.mProperties.coarseCollisionEpsilon),
		mFineCollisionDetector(
			mParentWorld.mProperties.coarseCollisionEpsilon,
			mParentWorld.mProperties.minFDifference, mParentWorld.mProperties.maxIterations,
			mParentWorld.mProperties.contactPrecision, mParentWorld.mProperties.contactSeparation,
			mParentWorld.mProperties.raycastPrecision
		)
	{
		mManifolds.reserve(mParentWorld.mProperties.maxCollidingRBs);
		mCollidersManifoldMap.reserve(mParentWorld.mProperties.maxCollidingRBs);
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
		mCoarseCollisionDetector.calculateCollisions([this](const ColliderPair& pair) {
			// Skip non updated Colliders
			if (pair.first->updated() || pair.second->updated()) {
				// Find a Manifold between the given colliders
				ColliderPair sortedPair = (pair.first <= pair.second)? pair : ColliderPair(pair.second, pair.first);
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


	void CollisionDetector::rayCastAll(
		const glm::vec3 origin, const glm::vec3& direction,
		const RayCastCallback& callback
	) {
		mCoarseCollisionDetector.calculateIntersections(origin, direction, [&](Collider* collider) {
			auto [intersects, rayCast] = mFineCollisionDetector.intersects(origin, direction, *collider);
			if (intersects) {
				callback(collider, rayCast);
			}
		});
	}


	std::pair<Collider*, RayCast> CollisionDetector::rayCastFirst(
		const glm::vec3 origin, const glm::vec3& direction
	) {
		Collider* collider = nullptr;
		RayCast rayCast;
		rayCast.distance = std::numeric_limits<float>::max();

		mCoarseCollisionDetector.calculateIntersections(origin, direction, [&](Collider* collider2) {
			auto [intersects, rayCast2] = mFineCollisionDetector.intersects(origin, direction, *collider2);
			if (intersects && (!collider || (rayCast2.distance < rayCast.distance))) {
				collider = collider2;
				rayCast = rayCast2;
			}
		});

		return { collider, rayCast };
	}

}
