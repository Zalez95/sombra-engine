#ifndef CONTACT_H
#define CONTACT_H

#include <glm/glm.hpp>

namespace physics {
	
	/**
	 * Struct Contact. It holds the data of a contact created as a result of
	 * a Collision
	 */
	struct Contact
	{
		/** The penetration depth of the Contact */
		float mPenetration;

		/** The position where the Contact is happening in world space */
		glm::vec3 mPosition;

		/** The normal direction of the Contact in world space */
		glm::vec3 mNormal;

		/** Creates a new Contact
		 *
		 * @param	penetration the penetration depth of the Contact
		 * @param	position the position where the Contact is located in world
		 *			space
		 * @param	normal the normal direction of the Contact in world
		 *			space
		 * @note	the normal must be normalized */
		Contact(
			float penetration,
			const glm::vec3& position, const glm::vec3& normal
		) : mPenetration(penetration), mPosition(position), mNormal(normal) {};

		/** Class destructor */
		~Contact() {};
	};

}

#endif		// CONTACT_H
