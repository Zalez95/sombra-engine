#include "fe/collision/AABB.h"
#include <algorithm>

namespace fe { namespace collision {

	bool AABB::overlaps(const AABB& other) const
	{
		AABB b1 = *this, b2 = other;

		if (b2.mMinimum.x < b1.mMinimum.x) { std::swap(b1, b2); }
		bool intersecX = (b1.mMaximum.x >= b2.mMinimum.x) && (b1.mMinimum.x <= b2.mMaximum.x);

		if (b2.mMinimum.y < b1.mMinimum.y) { std::swap(b1, b2); }
		bool intersecY = (b1.mMaximum.y >= b2.mMinimum.y) && (b1.mMinimum.y <= b2.mMaximum.y);

		if (b2.mMinimum.z < b1.mMinimum.z) { std::swap(b1, b2); }
		bool intersecZ = (b1.mMaximum.z >= b2.mMinimum.z) && (b1.mMinimum.z <= b2.mMaximum.z);

		return (intersecX && intersecY && intersecZ);
	}

}}
