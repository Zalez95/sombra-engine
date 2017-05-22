#include "Plane.h"

namespace physics {

	void Plane::setTransforms(const glm::mat4& transforms)
	{
		glm::vec3 position(transforms * glm::vec4());
		mNormal = glm::normalize(position);
		mDistance = glm::length(position);
	}

}
