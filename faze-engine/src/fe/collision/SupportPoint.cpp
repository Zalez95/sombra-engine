#include "fe/collision/SupportPoint.h"
#include "fe/collision/ConvexCollider.h"

namespace fe { namespace collision {

	SupportPoint::SupportPoint(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const glm::vec3& direction
	) {
		collider1.getFurthestPointInDirection( direction, mWorldPosition[0], mLocalPosition[0]);
		collider2.getFurthestPointInDirection(-direction, mWorldPosition[1], mLocalPosition[1]);
		mCSOPosition = mWorldPosition[0] - mWorldPosition[1];
	}


	bool SupportPoint::operator==(const SupportPoint& other) const
	{
		return (mWorldPosition[0] == other.mWorldPosition[0])
			&& (mWorldPosition[1] == other.mWorldPosition[1]);
	}

}}
