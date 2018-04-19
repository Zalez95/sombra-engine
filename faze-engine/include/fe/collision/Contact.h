#ifndef CONTACT_H
#define CONTACT_H

#include <glm/glm.hpp>

namespace fe { namespace collision {

	/**
	 * Class Contact. It holds the data of a contact created as a result of
	 * a Collision.
	 */
	class Contact
	{
	private:	// Attributes
		friend class EPACollisionDetector;

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
		/** Creates a new Contact */
		Contact() {};

		/** Class destructor */
		~Contact() {};

		/** @return	the penetration depth of the Contact */
		inline float getPenetration() const { return mPenetration; };

		/** @return	the normal direction of the Contact from the first Collider
		 * to to the second one in world space */
		inline glm::vec3 getNormal() const { return mNormal; };

		/** Returns the position of the Contact in world space relative to the
		 * given collider
		 *
		 * @param	collider the index of the collider (0 or 1)
		 * @return	the position in world space of the Contact */
		inline glm::vec3 getWorldPosition(int collider) const
		{ return mWorldPos[collider]; };

		/** Returns the position of the Contact in local space relative to the
		 * given collider
		 *
		 * @param	collider the index of the collider (0 or 1)
		 * @return	the position in local space of the Contact */
		inline glm::vec3 getLocalPosition(int collider) const
		{ return mLocalPos[collider]; };
	};

}}

#endif		// CONTACT_H
