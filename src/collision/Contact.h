#ifndef CONTACT_H
#define CONTACT_H

#include <cassert>
#include <glm/glm.hpp>

namespace collision {
	
	/**
	 * Class Contact. It holds the data of a contact created as a result of
	 * a Collision.
	 */
	class Contact
	{
	private:	// Attributes
		friend class FineCollisionDetector;

		/** The penetration depth of the Contact */
		float mPenetration;

		/** The direction along we should separate the intersecting bodies in
		 * world space */
		glm::vec3 mNormal;

		/** The coordinates of the Contact point relative to each of the
		 * Collider in world space */
		glm::vec3 mWorldPos[2];

		/** The coordinates of the Contact point relative to each of the
		 * Collider in local space */
		glm::vec3 mLocalPos[2];

	public:		// Functions
		/** Creates a new Contact
		 *
		 * @param	penetration the penetration depth of the Contact
		 * @param	normal the normal direction of the Contact in world
		 *			space
		 * @note	the normal must be normalized */
		Contact(float penetration, const glm::vec3& normal) :
			mPenetration(penetration), mNormal(normal) {};

		/** Class destructor */
		~Contact() {};

		/** @return	the penetration depth of the Contact */
		inline float getPenetration() const { return mPenetration; };

		/** @return	the normal direction of the Contact in world space */
		inline glm::vec3 getNormal() const { return mNormal; };
		
		/** Returns the position of the Contact in world space relative to the
		 * given collider
		 * 
		 * @param	collider the index of the collider (0 or 1)
		 * @return	the position in world space of the Contact */
		inline glm::vec3 getWorldPosition(size_t collider) const
		{
			assert(collider < 2 && "There are only two colliders");
			return mWorldPos[collider];
		};

		/** Returns the position of the Contact in local space relative to the
		 * given collider
		 * 
		 * @param	collider the index of the collider (0 or 1)
		 * @return	the position in local space of the Contact */
		inline glm::vec3 getLocalPosition(size_t collider) const
		{
			assert(collider < 2 && "There are only two colliders");
			return mLocalPos[collider];
		};
	};

}

#endif		// CONTACT_H
