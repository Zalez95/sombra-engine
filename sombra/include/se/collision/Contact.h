#ifndef CONTACT_H
#define CONTACT_H

#include <glm/glm.hpp>

namespace se::collision {

	/**
	 * Struct Contact. It holds the data of a contact created as a result of
	 * a Collision.
	 */
	struct Contact
	{
		/** The penetration depth of the Contact */
		float penetration;

		/** The normal direction along we should separate the intersecting
		 * Colliders in world space. It's a vector with an origin in the second
		 * contact point pointing outside the first Collider */
		glm::vec3 normal;

		/** The coordinates of the Contact point relative to each of the
		 * Collider in world space */
		glm::vec3 worldPosition[2];

		/** The coordinates of the Contact point relative to each of the
		 * Collider in local space */
		glm::vec3 localPosition[2];

		/** Creates a new Contact */
		Contact() : penetration(0.0f), normal(0.0f),
			worldPosition{ glm::vec3(0.0f), glm::vec3(0.0f) },
			localPosition{ glm::vec3(0.0f), glm::vec3(0.0f) } {};
	};

}

#endif		// CONTACT_H
