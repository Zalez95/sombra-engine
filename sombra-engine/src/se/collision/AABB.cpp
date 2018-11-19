#include <algorithm>
#include "se/collision/AABB.h"

namespace se::collision {

	bool overlaps(const AABB& aabb1, const AABB& aabb2)
	{
		const AABB *b1 = &aabb1, *b2 = &aabb2;
		if (b2->minimum.x < b1->minimum.x) { std::swap(b1, b2); }
		bool intersecX = (b1->maximum.x >= b2->minimum.x) && (b1->minimum.x <= b2->maximum.x);

		if (b2->minimum.y < b1->minimum.y) { std::swap(b1, b2); }
		bool intersecY = (b1->maximum.y >= b2->minimum.y) && (b1->minimum.y <= b2->maximum.y);

		if (b2->minimum.z < b1->minimum.z) { std::swap(b1, b2); }
		bool intersecZ = (b1->maximum.z >= b2->minimum.z) && (b1->minimum.z <= b2->maximum.z);

		return (intersecX && intersecY && intersecZ);
	}

}
