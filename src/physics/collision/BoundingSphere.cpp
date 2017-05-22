#include "BoundingSphere.h"

namespace physics {

	void BoundingSphere::setTransforms(const glm::mat4& transforms)
	{
		mCenter = glm::vec3(transforms[3]);
	}

}
