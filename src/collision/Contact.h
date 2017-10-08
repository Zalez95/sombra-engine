#ifndef CONTACT_H
#define CONTACT_H

#include <glm/glm.hpp>

namespace collision {
	
	/**
	 * Class Contact. It holds the data of a contact created as a result of
	 * a Collision
	 */
	class Contact
	{
	private:	// Attributes
		/** The penetration depth of the Contact */
		float mPenetration;

		/** The position where the Contact is happening in world space */
		glm::vec3 mPosition;

		/** The normal direction of the Contact in world space */
		glm::vec3 mNormal;

	public:		// Functions
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

		/** @return	the penetration depth of the Contact */
		inline float getPenetration() const { return mPenetration; };

		/** Sets the Contact penetration
		 * 
		 * @param	penetration the penetration depth of the Contact */
		inline void setPenetration(float penetration)
		{ mPenetration = penetration; };

		/** @return the position where the Contact is happening in world
		 *			space */
		inline glm::vec3 getPosition() const { return mPosition; };

		/** Sets the Contact position
		 * 
		 * @param	the position where the Contact is happening in world
		 *			space */
		inline void setPosition(const glm::vec3& position)
		{ mPosition = position; };

		/** @return	the normal direction of the Contact in world space */
		inline glm::vec3 getNormal() const { return mNormal; };

		/** Sets the normal direction of the Contact
		 * 
		 * @param	normal the normal direction of the Contact in world
		 *			space */
		inline void setNormal(const glm::vec3& normal) { mNormal = normal; };
	};

}

#endif		// CONTACT_H
