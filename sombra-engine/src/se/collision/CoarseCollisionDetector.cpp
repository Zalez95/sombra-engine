#include "se/collision/CoarseCollisionDetector.h"
#include "se/collision/Collider.h"

namespace se::collision {

	void CoarseCollisionDetector::submit(const Collider* collider)
	{
		mColliders.push_back({ collider, collider->getAABB() });
	}


	std::vector<CoarseCollisionDetector::ColliderPair> CoarseCollisionDetector::getIntersectingColliders()
	{
		std::vector<ColliderPair> ret;

		while (!mColliders.empty()) {
			for (std::size_t i = 1; i < mColliders.size(); ++i) {
				if ( overlaps(mColliders[0].aabb, mColliders[i].aabb) ) {
					ret.push_back(std::make_pair(mColliders[0].collider, mColliders[i].collider));
				}
			}

			mColliders.pop_front();
		}

		return ret;
	}

}
