#include "se/physics/collision/CoarseCollisionDetector.h"
#include "se/physics/collision/Collider.h"
#include "AABBAVLTree.h"

namespace se::physics {

	CoarseCollisionDetector::CoarseCollisionDetector(float epsilon) :
		mEpsilon(epsilon), mAABBTree(std::make_unique<AABBAVLTree<std::size_t>>()) {}


	CoarseCollisionDetector::~CoarseCollisionDetector() {}


	void CoarseCollisionDetector::add(Collider* collider)
	{
		auto itCollider = mColliders.emplace(ColliderData{ collider, 0 });
		itCollider->nodeId = mAABBTree->addNode(collider->getAABB(), itCollider.getIndex());
	}


	void CoarseCollisionDetector::processColliders(const ColliderCallback& callback) const
	{
		for (const ColliderData& cData : mColliders) {
			callback(cData.collider);
		}
	}


	void CoarseCollisionDetector::remove(Collider* collider)
	{
		auto itCollider = std::find_if(mColliders.begin(), mColliders.end(), [&](const ColliderData& cData) {
			return cData.collider == collider;
		});
		if (itCollider != mColliders.end()) {
			mAABBTree->removeNode(itCollider->nodeId);
			mColliders.erase(itCollider);
		}
	}


	void CoarseCollisionDetector::update()
	{
		for (auto itCollider = mColliders.begin(); itCollider != mColliders.end(); ++itCollider) {
			if (itCollider->collider->updated()) {
				mAABBTree->removeNode(itCollider->nodeId);
				itCollider->nodeId = mAABBTree->addNode(itCollider->collider->getAABB(), itCollider.getIndex());
			}
		}
	}


	void CoarseCollisionDetector::calculateCollisions(const CollisionCallback& callback) const
	{
		mAABBTree->calculateAllOverlaps(mEpsilon, [&](std::size_t nodeId1, std::size_t nodeId2) {
			std::size_t colliderIndex1 = mAABBTree->getNodeUserData(nodeId1);
			std::size_t colliderIndex2 = mAABBTree->getNodeUserData(nodeId2);
			callback(mColliders[colliderIndex1].collider, mColliders[colliderIndex2].collider);
		});
	}


	void CoarseCollisionDetector::calculateIntersections(const Ray& ray, const ColliderCallback& callback) const
	{
		mAABBTree->calculateIntersectionsWith(ray, mEpsilon, [&](std::size_t nodeId) {
			std::size_t colliderIndex = mAABBTree->getNodeUserData(nodeId);
			callback(mColliders[colliderIndex].collider);
		});
	}

}
