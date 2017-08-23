#include "CoarseCollisionDetector.h"
#include <algorithm>
#include "Collider.h"

namespace physics {

	void CoarseCollisionDetector::submit(const Collider* collider)
	{
		mColliders.push_back(collider);
	}


	std::vector<CoarseCollisionDetector::ColliderPair> CoarseCollisionDetector::getIntersectingColliders()
	{
		std::vector<ColliderPair> ret;

		for (const Collider* collider : mColliders) {
			mAABBs.push_back( collider->getAABB() );
		}

		while ( !mAABBs.empty() ) {
			for (size_t i = 1; i < mAABBs.size(); ++i) {
				AABB b1 = mAABBs[0], b2 = mAABBs[i];

				if (b2.mMinimum.x < b1.mMinimum.x) { std::swap(b1, b2); }
				bool intersecX = (b1.mMaximum.x > b2.mMinimum.x) && (b1.mMinimum.x < b2.mMaximum.x);

				if (b2.mMinimum.y < b1.mMinimum.y) { std::swap(b1, b2); }
				bool intersecY = (b1.mMaximum.y > b2.mMinimum.y) && (b1.mMinimum.y < b2.mMaximum.y);

				if (b2.mMinimum.z < b1.mMinimum.z) { std::swap(b1, b2); }
				bool intersecZ = (b1.mMaximum.z > b2.mMinimum.z) && (b1.mMinimum.z < b2.mMaximum.z);

				if (intersecX && intersecY && intersecZ) {
					ret.push_back( std::make_pair(mColliders[0], mColliders[i]) );
				}
			}

			mAABBs.pop_front();
			mColliders.pop_front();
		}

		return ret;
	}

}

