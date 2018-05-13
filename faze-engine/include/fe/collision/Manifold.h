#ifndef MANIFOLD_H
#define MANIFOLD_H

#include <vector>
#include "Contact.h"

namespace fe { namespace collision {

	class Collider;


	/**
	 * Class Manifold, a contact manifold holds all the Contacs between two
	 * Colliders. A Manifold can store up to 4 contacts.
	 */
	class Manifold
	{
	private:	// Attributes
		friend class FineCollisionDetector;

		/** All the Contacs the the Contact Manifold can hold */
		std::vector<Contact> mContacts;

		/** The colliders of the Manifold */
		const Collider* mColliders[2];

	public:		// Functions
		/** Creates a new Manifold
		 *
		 * @param	c1 a pointer to the first Collider of the Manifold
		 * @param	c2 a pointer to the second Collider of the Manifold */
		Manifold(const Collider* c1, const Collider* c2) :
			mColliders{ c1, c2 } {};

		/** Class destructor */
		~Manifold() {};

		/** @return	all the Contacs that the Manifold holds */
		inline std::vector<Contact> getContacts() const
		{ return mContacts; };

		/** Returns a pointer to the requested collider
		 *
		 * @param	second the flag used to select the collider to return
		 * @return	a pointer to the second Collider if the second flag is
		 *			true, the first one otherwise */
		inline const Collider* getCollider(bool second) const
		{ return (second)? mColliders[1] : mColliders[0]; };
	};

}}

#endif		// MANIFOLD_H
