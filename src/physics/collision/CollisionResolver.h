#ifndef COLLISION_RESOLVER_H
#define COLLISION_RESOLVER_H

#include "Contact.h"
#include "../RigidBody.h"

namespace physics {

	/** Class CollisionResolver */
	class CollisionResolver
	{
	public:		// Functions
		/** Creates a new CollisionResolver */
		CollisionResolver() {};

		/** Class destructor */
		~CollisionResolver() {};

		void solve(Contact* contact, RigidBody* rb1, RigidBody* rb2)
		{
			// 1. Create an orthonormal basis with the Contact normal as the X axis
			//			

			// 2. Create the basis matrix from the vector of the basis
			glm::mat3 contactMatrix = contact->getContactMatrix();


		};
	};

}

#endif		// COLLISION_RESOLVER_H
