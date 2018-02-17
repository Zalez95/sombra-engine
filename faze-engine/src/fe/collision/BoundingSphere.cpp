#include "fe/collision/BoundingSphere.h"
#include <cassert>

namespace fe { namespace collision {

	BoundingSphere::BoundingSphere(float radius) :
		mRadius(radius),
		mTransformsMatrix(1.0f), mInverseTransformsMatrix(1.0f)
	{
		assert(radius >= 0 && "The radius of the sphere can't be smaller than zero.");
	}


	void BoundingSphere::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mInverseTransformsMatrix = glm::inverse(transforms);
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
		pointWorld = center + mRadius * glm::normalize(direction);
		pointLocal = mInverseTransformsMatrix * glm::vec4(pointWorld, 1.0f);
	}

}}
