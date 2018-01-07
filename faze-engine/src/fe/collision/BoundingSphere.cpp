#include "fe/collision/BoundingSphere.h"
#include <cassert>
#include <glm/gtx/component_wise.hpp>

namespace fe { namespace collision {

	BoundingSphere::BoundingSphere(float radius) : mRadius(radius)
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
		pointWorld = center + mRadius * direction;
		pointLocal = mInverseTransformsMatrix * glm::vec4(pointWorld, 1.0f);
	}

}}
