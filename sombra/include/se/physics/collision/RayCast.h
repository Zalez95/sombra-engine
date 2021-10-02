#ifndef RAY_CAST_H
#define RAY_CAST_H

#include <glm/glm.hpp>

namespace se::physics {

	/**
	 * Struct RayCast, holds all the information about a ray hit on a Collider
	 */
	struct RayCast
	{
		/** The distance from the ray origin */
		float distance;

		/** The world coordinates of the contact point */
		glm::vec3 contactPointWorld;

		/** The local coordinates of the contact point */
		glm::vec3 contactPointLocal;

		/** The normal at the surface of contact */
		glm::vec3 contactNormal;
	};

}

#endif		// RAY_CAST_H
