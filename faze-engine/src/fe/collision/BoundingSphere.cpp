#include "fe/collision/BoundingSphere.h"
#include <cassert>

namespace fe { namespace collision {

	BoundingSphere::BoundingSphere(float radius) : mRadius(radius)
	{
		assert(radius >= 0 && "The radius of the sphere can't be smaller than zero.");
	}


	void BoundingSphere::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
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
		glm::vec3 center = getCenter();
		glm::vec3 frontLocal = glm::vec3(0, 0, 1);
		glm::vec3 frontWorld = glm::vec3(glm::vec4(frontLocal, 0) * mTransformsMatrix);

		pointWorld = center + mRadius * glm::normalize(direction);
		pointLocal = pointWorld * frontLocal / frontWorld;
	}

}}
