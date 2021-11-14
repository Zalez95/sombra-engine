#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

namespace se::physics {

	/**
	 * Struct Ray, it holds all the data needed for performing a ray cast
	 */
	struct Ray
	{
		/** The origin of the Ray */
		glm::vec3 origin = {};

		/** The direction of the Ray */
		glm::vec3 direction = {};

		/** The inverse of @see direction component wise */
		glm::vec3 invertedDirection = {};

		/** Creates a new Ray
		 *
		 * @param	origin the origin of the new Ray
		 * @param	direction the direction of the new Ray */
		Ray(const glm::vec3& origin, const glm::vec3& direction) :
			origin(origin), direction(direction),
			invertedDirection(
				1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z
			) {};
	};


	/**
	 * Struct RayHit, holds all the information about a ray hit on a Collider
	 */
	struct RayHit
	{
		/** The distance from the ray origin */
		float distance = 0.0f;

		/** The world coordinates of the contact point */
		glm::vec3 contactPointWorld = {};

		/** The local coordinates of the contact point */
		glm::vec3 contactPointLocal = {};

		/** The normal at the surface of contact */
		glm::vec3 contactNormal = {};
	};

}

#endif		// RAY_H
