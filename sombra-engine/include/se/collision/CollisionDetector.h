#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <set>
#include <map>
#include <memory>
#include "Manifold.h"
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"

namespace se::collision {

	class Collider;


	/**
	 * Class CollisionDetector, it's the main class used to detect collisions
	 * between Colliders and calculate the Manifolds and its Contact data.
	 *
	 * The collision detection it splitted in two phases, one coarse collision
	 * detection where we're going to check which colliders could be
	 * intersecting by its AABB intersections, and one fine collision detection
	 * where we will check if they are colliding by the actual coordinates of
	 * the Colliders and calculate the Contact data.
	 */
	class CollisionDetector
	{
	private:	// Nested types
		using ColliderPair = std::pair<const Collider*, const Collider*>;

	private:	// Attributes
		static constexpr float kMinFDifference		= 0.00001f;
		static constexpr float kContactPrecision	= 0.0000001f;
		static constexpr float kContactSeparation	= 0.00001f;

		/** The CoarseCollisionDetector of the CollisionDetector. We will use
		 * it to check what Colliders are intersecting in the broad phase of
		 * the collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the CollisionDetector. We will use
		 * it to generate all the contact data */
		FineCollisionDetector mFineCollisionDetector;

		/** All the Colliders to check */
		std::set<const Collider*> mColliders;

		/** Maps a pair of Colliders with the the Manifold of it's collision */
		std::map<ColliderPair, Manifold> mMapCollidersManifolds;

		/** The cached pointers to all the current contact Manifolds of all
		 * the detected collisions */
		std::set<Manifold*> mManifolds;

	public:		// Functions
		/** Creates a new CollisionDetector */
		CollisionDetector() :
			mFineCollisionDetector(
				kMinFDifference, kContactPrecision,
				kContactSeparation
			) {};

		/** @return all the contact manifolds of the detected collisions */
		inline std::set<Manifold*> getCollisionManifolds() const
		{ return mManifolds; };

		/** Adds the given Collider to the CollisionDetector so it will
		 * check if it collides with the other Colliders
		 *
		 * @param	collider a pointer to the Collider that we want to add */
		void addCollider(const Collider* collider);

		/** Removes the given Collider from the CollisionDetector so it wont't
		 * check if it collides with the other Colliders
		 *
		 * @param	collider a pointer to the Collider that we want to
		 * 			remove */
		void removeCollider(const Collider* collider);

		/** Calculates all the collisions that can be currently happening
		 * between the Colliders added to the CollisionDetector */
		void update();
	private:
		/** Searchs a Manifold already stored with by the given colliders, and
		 * if it doesn't found one, creates a new one
		 *
		 * @param	c1 a pointer to the first collider to search for the
		 *			manifold
		 * @param	c2 a pointer to the second collider to search for the
		 *			manifold
		 * @return	a pointer to the manifold */
		Manifold* getManifold(const Collider* c1, const Collider* c2);
	};

}

#endif		// COLLISION_DETECTOR_H