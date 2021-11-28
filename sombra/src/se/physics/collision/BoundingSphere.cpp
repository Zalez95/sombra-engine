#include <cassert>
#include "se/physics/collision/BoundingSphere.h"

namespace se::physics {

	BoundingSphere::BoundingSphere(float radius) :
		mRadius(0.0f), mTransformsMatrix(1.0f), mInverseTransformsMatrix(1.0f)
	{
		setRadius(radius);
	}


	void BoundingSphere::setRadius(float radius)
	{
		assert(radius >= 0.0f && "The radius of the sphere can't be smaller than zero.");
		mRadius = radius;
		mUpdated = true;
	}


	void BoundingSphere::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mInverseTransformsMatrix = glm::inverse(transforms);
		mUpdated = true;
	}


	AABB BoundingSphere::getAABB() const
	{
		glm::vec3 center = getCenter();
		return { center - glm::vec3(mRadius), center + glm::vec3(mRadius) };
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

}
