#include "fe/collision/CoarseCollisionDetector.h"
#include "fe/collision/Collider.h"

namespace fe { namespace collision {

	void CoarseCollisionDetector::submit(const Collider* collider)
	{
		mColliders.push_back(collider);
		mAABBs.push_back(collider->getAABB());
	}


	std::set<CoarseCollisionDetector::ColliderPair> CoarseCollisionDetector::getIntersectingColliders()
	{
		std::set<ColliderPair> ret;

		while (!mAABBs.empty()) {
			for (size_t i = 1; i < mAABBs.size(); ++i) {
				if ( mAABBs[0].overlaps(mAABBs[i]) ) {
					ret.insert(std::make_pair(mColliders[0], mColliders[i]));
				}
			}

			mAABBs.pop_front();
			mColliders.pop_front();
		}

		return ret;
	}

}}
