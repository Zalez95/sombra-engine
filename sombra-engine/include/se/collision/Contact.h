#ifndef CONTACT_H
#define CONTACT_H

#include <glm/glm.hpp>

namespace se::collision {

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
		glm::vec3 mWorldPosition[2];

		/** The coordinates of the Contact point relative to each of the
		 * Collider in local space */
		glm::vec3 mLocalPosition[2];

	public:		// Functions
		/** Creates a new Contact */
		Contact() : mPenetration(0.0f), mNormal(0.0f),
			mWorldPosition{ glm::vec3(0.0f), glm::vec3(0.0f) },
			mLocalPosition{ glm::vec3(0.0f), glm::vec3(0.0f) } {};

		/** Class destructor */
		~Contact() {};

		/** @return	the penetration depth of the Contact */
		inline float getPenetration() const { return mPenetration; };

		/** @return	the normal direction of the Contact in world space pointing
		 *			from the first Collider to the second one. It's the
		 *			direction along we should separate the Colliders */
		inline glm::vec3 getNormal() const { return mNormal; };

		/** Returns the position of the Contact in world space relative to the
		 * requested Collider
		 *
		 * @param	second the flag used to select the world position to return
		 * @return	the position in world space of the Contact relative to the
		 *			second Collider if the flag is true, or relative to the
		 *			first one of the flag is false */
		inline glm::vec3 getWorldPosition(bool second) const
		{ return (second)? mWorldPosition[1] : mWorldPosition[0]; };

		/** Returns the position of the Contact in local space relative to the
		 * requested Collider
		 *
		 * @param	second the flag used to select the local position to return
		 * @return	the position in local space of the Contact relative to the
		 *			second Collider if the flag is true, or relative to the
		 *			first one of the flag is false */
		inline glm::vec3 getLocalPosition(bool second) const
		{ return (second)? mLocalPosition[1] : mLocalPosition[0]; };
	};

}

#endif		// CONTACT_H
