#include "BoundingSphere.h"
#include "AABB.h"

namespace collision {

	void BoundingSphere::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix	= transforms;
	}


	AABB BoundingSphere::getAABB() const
	{
		glm::vec3 center = getCenter();
		return AABB(center - glm::vec3(mRadius), center + glm::vec3(mRadius));
	}


	void BoundingSphere::getFurthestPointInDirection(
		const glm::vec3& direction,
		glm::vec3& pointWorld, glm::vec3& pointLocal
	) const
	{
		pointWorld = getCenter() + mRadius * glm::normalize(direction);
		pointLocal = glm::vec3();// TODO: mRadius * glm::axis(glm::quat(direction) * glm::inverse(mOrientation));
	}

}
