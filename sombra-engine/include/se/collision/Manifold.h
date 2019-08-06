#ifndef MANIFOLD_H
#define MANIFOLD_H

#include "Contact.h"
#include "../utils/FixedVector.h"

namespace se::collision {

	class Collider;

	/** The different states in which a Manifold can be */
	enum class ManifoldState
	{
		Disjoint,
		Intersecting
	};


	/**
	 * Struct Manifold, a contact manifold that holds all the Contacs between
	 * two Colliders. A Manifold can store up to 4 contacts.
	 */
	struct Manifold
	{
		/** The maximum number of Contacts in the Manifold */
		static constexpr std::size_t kMaxContacts = 4;

		/** The current state of the Manifold */
		ManifoldState state;

		/** The colliders of the Manifold */
		const Collider* colliders[2];

		/** All the Contacs the Contact Manifold can hold */
		utils::FixedVector<Contact, kMaxContacts> contacts;

		/** If the Manifold has been updated by the CollisionWorld or not */
		bool updated;

		/** Creates a new Manifold
		 *
		 * @param	c1 a pointer to the first Collider of the Manifold
		 * @param	c2 a pointer to the second Collider of the Manifold
		 * @param	state the initial state of the Manifold */
		Manifold(
			const Collider* c1, const Collider* c2,
			ManifoldState state = ManifoldState::Disjoint
		) : state(state), colliders{ c1, c2 }, updated(true) {};
	};

}

#endif		// MANIFOLD_H
