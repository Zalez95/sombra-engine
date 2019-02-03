#include "se/collision/SupportPoint.h"
#include "se/collision/ConvexCollider.h"

namespace se::collision {

	SupportPoint::SupportPoint(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const glm::vec3& direction
	) {
		collider1.getFurthestPointInDirection( direction, mWorldPosition[0], mLocalPosition[0]);
		collider2.getFurthestPointInDirection(-direction, mWorldPosition[1], mLocalPosition[1]);
		mCSOPosition = mWorldPosition[0] - mWorldPosition[1];
	}


	bool operator==(const SupportPoint& sp1, const SupportPoint& sp2)
	{
		return (sp1.mWorldPosition[0] == sp2.mWorldPosition[0])
			&& (sp1.mWorldPosition[1] == sp2.mWorldPosition[1]);
	}

}
