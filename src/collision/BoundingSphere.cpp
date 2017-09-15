#include "BoundingSphere.h"
#include "AABB.h"

namespace collision {

	void BoundingSphere::setTransforms(const glm::mat4& transforms)
	{
		mCenter = glm::vec3(transforms[3]);
	}


	AABB BoundingSphere::getAABB() const
	{
		return AABB(mCenter - glm::vec3(mRadius), mCenter + glm::vec3(mRadius));
	}


	glm::vec3 BoundingSphere::getFurthestPointInDirection(
		const glm::vec3& direction
	) const
	{
		return (mCenter + mRadius * glm::normalize(direction));
	}

}
