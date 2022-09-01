#include <algorithm>
#include "se/physics/LogWrapper.h"
#include "se/physics/RigidBodyWorld.h"
#include "se/physics/collision/CollisionDetector.h"

namespace se::physics {

	CollisionDetector::CollisionDetector(RigidBodyWorld& parentWorld) :
		mParentWorld(parentWorld),
		mCoarseCollisionDetector(mParentWorld.getProperties().collisionProperties.coarseEpsilon),
		mFineCollisionDetector(
			mParentWorld.getProperties().collisionProperties.coarseEpsilon,
			mParentWorld.getProperties().collisionProperties.minFDifference,
			mParentWorld.getProperties().collisionProperties.maxIterations,
			mParentWorld.getProperties().collisionProperties.contactPrecision,
			mParentWorld.getProperties().collisionProperties.contactSeparation,
			mParentWorld.getProperties().collisionProperties.raycastPrecision
		)
	{
		mCoarseCollidersColliding.reserve(mParentWorld.getProperties().collisionProperties.maxCollidersIntersecting);
		mManifolds.reserve(mParentWorld.getProperties().collisionProperties.maxCollidersIntersecting);
		mCollidersManifoldMap.reserve(mParentWorld.getProperties().collisionProperties.maxCollidersIntersecting);
	}


	void CollisionDetector::addCollider(Collider* collider)
	{
		std::scoped_lock lck(mMutex);
		mCoarseCollisionDetector.add(collider);
	}


	void CollisionDetector::removeCollider(Collider* collider)
	{
		std::scoped_lock lck(mMutex);
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
		std::scoped_lock lck(mMutex);

		// Clean old non intersecting Manifolds
		for (auto it = mCollidersManifoldMap.begin(); it != mCollidersManifoldMap.end();) {
			if (!mManifolds[it->second].state[Manifold::State::Intersecting]) {
				mManifolds.erase( mManifolds.begin().setIndex(it->second) );
				it = mCollidersManifoldMap.erase(it);
			}
			else {
				// Set the remaining Manifolds' state to not intersecting, so
				// the state of those skipped by the coarse collision detection
				// are also updated
				mManifolds[it->second].state.reset(Manifold::State::Intersecting);
				mManifolds[it->second].state.set(Manifold::State::Updated);
				++it;
			}
		}

		broadCollisionDetection();
		narrowCollisionDetection();

		// Notify the ICollisionListeners
		for (ICollisionListener* listener : mListeners) {
			for (auto& manifold : mManifolds) {
				listener->onCollision(manifold);
			}
		}
	}


	void CollisionDetector::addListener(ICollisionListener* listener)
	{
		SPHYS_DEBUG_LOG(mParentWorld) << "Adding listener " << listener;

		if (listener) {
			std::scoped_lock lck(mMutex);
			mListeners.push_back(listener);
		}

		SPHYS_DEBUG_LOG(mParentWorld) << "Added listener " << listener;
	}


	void CollisionDetector::removeListener(ICollisionListener* listener)
	{
		SPHYS_DEBUG_LOG(mParentWorld) << "Removing listener " << listener;

		std::scoped_lock lck(mMutex);
		mListeners.erase(
			std::remove(mListeners.begin(), mListeners.end(), listener),
			mListeners.end()
		);

		SPHYS_DEBUG_LOG(mParentWorld) << "Removed listener " << listener;
	}


	void CollisionDetector::rayCastAll(const Ray& ray, const RayCastCallback& callback)
	{
		std::scoped_lock lck(mMutex);
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

		std::scoped_lock lck(mMutex);
		mCoarseCollisionDetector.calculateIntersections(ray, [&](Collider* collider2) {
			auto [intersects, rayHit2] = mFineCollisionDetector.intersects(ray, *collider2);
			if (intersects && (!collider || (rayHit2.distance < rayHit.distance))) {
				collider = collider2;
				rayHit = rayHit2;
			}
		});

		return { collider, rayHit };
	}

// Private functions
	void CollisionDetector::broadCollisionDetection()
	{
		mCoarseCollisionDetector.update();

		// Store all the Colliders intersecting in mCoarseCollidersColliding
		mCoarseCollidersColliding.clear();
		mCoarseCollisionDetector.calculateCollisions([this](Collider* collider1, Collider* collider2) {
			// Skip non updated Colliders and Colliders without any common layer
			if ((collider1->updated() || collider2->updated())
				&& ((collider1->getLayers() & collider2->getLayers()).any())
			) {
				mCoarseCollidersColliding.emplace_back(collider1, collider2);
			}
		});

		// Reset the updated state of all the Colliders
		mCoarseCollisionDetector.processColliders([](Collider* collider) {
			collider->resetUpdatedState();
		});
	}


	void CollisionDetector::narrowCollisionDetection()
	{
		// Execute singleNarrowCollision with the pairs stored in
		// mCoarseCollidersColliding in parallel
		std::size_t nThreads = mParentWorld.getProperties().numThreads;
		std::size_t pairsPerThread = mCoarseCollidersColliding.size() / nThreads;
		std::vector<std::future<std::vector<Manifold>>> threadFutures(nThreads);

		for (std::size_t iThread = 0; iThread < nThreads; ++iThread) {
			threadFutures[iThread] = mParentWorld.getThreadPool().async([=]() {
				std::size_t iStart = iThread * pairsPerThread;
				std::size_t iEnd = (iThread < nThreads - 1)?
					(iThread + 1) * pairsPerThread :
					mCoarseCollidersColliding.size();

				std::vector<Manifold> newManifolds;
				for (std::size_t i = iStart; i < iEnd; ++i) {
					singleNarrowCollision(mCoarseCollidersColliding[i], newManifolds);
				}
				return newManifolds;
			});
		}

		// The new manifolds doesn't repeat and their colliders are
		// already sorted
		std::vector<Manifold> newManifolds;
		for (auto& future : threadFutures) {
			std::vector<Manifold> thNewManifolds = future.get();
			newManifolds.insert(newManifolds.end(), thNewManifolds.begin(), thNewManifolds.end());
		}

		for (auto& newManifold : newManifolds) {
			if (mManifolds.size() < mManifolds.capacity()) {
				auto itManifold = mManifolds.emplace(std::move(newManifold));
				mCollidersManifoldMap.emplace(
					std::piecewise_construct,
					std::forward_as_tuple(itManifold->colliders[0], itManifold->colliders[1]),
					std::forward_as_tuple(itManifold.getIndex())
				);
			}
			else {
				SPHYS_DEBUG_LOG(mParentWorld) << "Can't create more Manifolds";
				break;
			}
		}
	}


	void CollisionDetector::singleNarrowCollision(const ColliderPair& pair, std::vector<Manifold>& newManifolds)
	{
		// Find a Manifold between the colliders
		ColliderPair sortedPair = (pair.first <= pair.second)? pair : ColliderPair(pair.second, pair.first);

		auto itPairManifold = mCollidersManifoldMap.find(sortedPair);
		if (itPairManifold != mCollidersManifoldMap.end()) {
			// Set the Manifold back to its old state (if we are at this
			// stage it was Intersecting in the previous frame)
			mManifolds[itPairManifold->second].state.set(Manifold::State::Intersecting);
			mManifolds[itPairManifold->second].state.reset(Manifold::State::Updated);

			// Update the Manifold data
			mFineCollisionDetector.collide(mManifolds[itPairManifold->second]);
		}
		else {
			// Create a new Manifold
			Manifold manifold(sortedPair.first, sortedPair.second);
			if (mFineCollisionDetector.collide(manifold)) {
				newManifolds.push_back(std::move(manifold));
			}
		}
	}

}
