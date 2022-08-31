#include <algorithm>
#include "se/graphics/core/GraphicsMath.h"

namespace se::graphics {

	float signedDistancePlanePoint(const glm::vec4& plane, const glm::vec3& point)
	{
		return glm::dot(glm::vec3(plane), point) + plane.w;
	}


	std::pair<glm::vec3, glm::vec3> getBoundsWorld(
		const glm::vec3& localMin, const glm::vec3& localMax,
		const glm::mat4& worldMatrix
	) {
		std::array<glm::vec3, 8> localAABBVertices = {
			glm::vec3(localMin.x, localMin.y, localMin.z),
			glm::vec3(localMin.x, localMin.y, localMax.z),
			glm::vec3(localMin.x, localMax.y, localMin.z),
			glm::vec3(localMin.x, localMax.y, localMax.z),
			glm::vec3(localMax.x, localMin.y, localMin.z),
			glm::vec3(localMax.x, localMin.y, localMax.z),
			glm::vec3(localMax.x, localMax.y, localMin.z),
			glm::vec3(localMax.x, localMax.y, localMax.z)
		};

		glm::vec3 worldMin, worldMax;
		worldMin = worldMax = worldMatrix * glm::vec4(localAABBVertices[0], 1.0f);
		for (std::size_t i = 1; i < 8; ++i) {
			glm::vec3 vw = worldMatrix * glm::vec4(localAABBVertices[i], 1.0f);
			worldMin = glm::min(worldMin, vw);
			worldMax = glm::max(worldMax, vw);
		}

		return { worldMin, worldMax };
	}


	void decompose(
		const glm::mat4& transforms,
		glm::vec3& translation, glm::quat& rotation, glm::vec3& scale
	) {
		translation = transforms[3];
		scale = glm::vec3(glm::length(transforms[0]), glm::length(transforms[1]), glm::length(transforms[2]));
		rotation = glm::quat_cast( glm::mat3(transforms[0] / scale.x, transforms[1] / scale.y, transforms[2] / scale.z) );
	}

}
