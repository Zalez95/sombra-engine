#ifndef COLLISION_RESOLVER_H
#define COLLISION_RESOLVER_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "collision/Contact.h"

#define RESTITUTION		0.3f
#define ANGULAR_LIMIT	0.2f

namespace physics {

	class RigidBody;


	/**
	 * Class CollisionResolver
	 */
	class CollisionResolver
	{
	private:	// Nested types
		/** Struct ContactData, it holds a Contact with the RigidBodies that
		 * created a collision, it also holds other data calculated during the
		 * collision resolution steps */
		struct ContactData {
			/** The Contact data of the intersection between the RigidBodies */
			Contact mContact;

			/** The RigidBodies whose collision generated the current
			 * Contact */
			RigidBody* mContactBodies[2];

			/** The matrix used to transform the coordinates from Contact
			 * space to World space */
			glm::mat3 mContactToWorldMatrix;

			/** The position of the Contact relative to both RigidBodies */
			glm::vec3 mRelativePositions[2];

			/** The velocity of the Contact relactive to both RigidBodies */
			glm::vec3 mRelativeVelocities[2];

			/** The change in velocity that we have to apply to both
			 * RigidBodies */
			glm::vec3 mVelocityChange[2];

			/** The change in angular velocity that we have to apply to both
			 * RigidBodies */
			glm::vec3 mRotationChange[2];

			/** The change in position that we have to apply to both
			 * RigidBodies */
			glm::vec3 mPositionChange[2];

			/** The change in orientation that we have to apply to both
			 * RigidBodies */
			glm::quat mOrientationChange[2];

			/** Creates a new ContactData
			 * 
			 * @param	contact the Contact of the ContactData
			 * @param	rb1 a pointer to the first RigidBody of the ContactData
			 * @param	rb1 a pointer to the second RigidBody of the
			 *			ContactData */
			ContactData(Contact& contact, RigidBody* rb1, RigidBody* rb2) :
				mContact(contact), mContactBodies{rb1, rb2} {};

			/** Destructor */
			~ContactData() {};
		};

    private:    // Attributes
        /** The contacts that the CollisionResolver must resolve */
        std::vector<ContactData> mContacts;

	public:		// Functions
		/** Creates a new CollisionResolver */
		CollisionResolver() {};

		/** Class destructor */
		~CollisionResolver() {};

        /** Adds the given Contact to the queue of Contacts to resolve
         *
         * @param   contact the Contact that we want to add
		 * @param	rb1 the first RigidBody that collided
		 * @param	rb2 the second RigidBody that collided */
        void addContact(Contact& contact, RigidBody* rb1, RigidBody* rb2);

        /** Resolves all the collisions submited to the CollisionResolver
		 *
		 * @param	delta the elapsed time since the last Update of the
		 * 			CollisionResolver */
		void resolve(float delta);
	private:
        /** Precalculates all the needed data of the given ContactData for the
		 * next steps in the collision resolution
         * 
         * @param   contactData the ContactData to prepare */
        void prepareContact(ContactData& contactData);

		/** Calculates the matrix used to transform the coordinates from
		 * Contact space to World space
		 * 
		 * @param	contact the Contact whose coordinates will be used to
		 *			generate the matrix
		 * @return	the matrix that transforms the coordinates from Contact
		 *			Space to World Space */
		glm::mat3 getContactToWorldMatrix(Contact& contact) const;

		/** Calculate the change in the position and orientation of the
		 * RigidBodies due to the collision using nonlinear projection
         * 
         * @param   contactData the ContactData whose RigidBodies' positions
		 *			have to change */
		void calculatePositionChanges(ContactData& contactData);

		/** Calculates the change in the linear and angular velocity of the
         * RigidBodies due to the collision
         * 
         * @param   contactData the ContactData whose RigidBodies' velocities
		 *			have to change
		 * @param	delta the elapsed time since the last Update of the
		 * 			CollisionResolver */
		void calculateVelocityChanges(ContactData& contactData, float delta);

		/** Updates the penetration of the contacts that have at least one
		 * RigidBody equal to one of RigidBodies in the given ContactData
		 * 
         * @param   contactData the ContactData whose RigidBodies' has been
		 *			changed */
		void updateOtherContacts(ContactData& contactData);
	};

}

#endif		// COLLISION_RESOLVER_H
