#ifndef CONTACT_H
#define CONTACT_H

#include <vector>
#include <glm/glm.hpp>

namespace physics {
	
	/**
	 * Class Contact. It holds the data of a contact created as a result of
	 * a Collision
	 */
	class Contact
	{
	private:	// Attributes
		/** The penetration depth of the Contact */
		float mPenetration;

		/** The point where the Contact is happening in world space */
		glm::vec3 mPoint;

		/** The normal direction of the Contact in wolrd space */
		glm::vec3 mNormal;

	public:		// Functions
		/** Creates a new Contact
		 *
		 * @param	penetration the penetration depth of the Contact
		 * @param	point the point where the Contact is happening in world
		 *			space
		 * @param	normal the normal direction of the Contact in world
		 *			space
		 * @note	the normal must be normalized */
		Contact(
			float penetration,
			const glm::vec3& point, const glm::vec3& normal
		) : mPenetration(penetration),
			mPoint(point), mNormal(normal) {};

		/** Class destructor */
		~Contact() {};

		/** @return	the penetration depth of the Contact */
		inline float getPenetration() const { return mPenetration; };

		/** @return	the position where the Contact is happening in world
		 *			space */
		inline glm::vec3 getPoint() const { return mPoint; };

		/** @return	the normal direction of the Contact in world space */
		inline glm::vec3 getNormalDirection() const { return mNormal; };

		/** @return a matrix used to transform the coordinates from World
		 * space to Contact space
		 * @note	we generate the normal each time this functions is called
		 *			with the normal of the Contact */
		glm::mat3 getContactMatrix() const;
	};

}

#endif		// CONTACT_H
