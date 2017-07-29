#include "Plane.h"
#include "AABB.h"

namespace physics {

	void Plane::setTransforms(const glm::mat4& transforms)
	{
		glm::vec3 position(transforms * glm::vec4());
		mNormal = glm::normalize(position);
		mDistance = glm::length(position);
	}


	AABB Plane::getAABB() const
	{
		return AABB(glm::vec3(), glm::vec3());
	}

}
