#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <set>
#include <map>
#include <memory>
#include "Manifold.h"
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"

namespace fe { namespace collision {

	class Collider;


	/**
	 * Class CollisionDetector, TODO
	 */
	class CollisionDetector
	{
	private:	// Nested types
		typedef std::pair<const Collider*, const Collider*> ColliderPair;
		
	private:	// Attributes
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
		CollisionDetector() {};

		/** Class destructor */
		~CollisionDetector() {};

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
	};

}}

#endif		// COLLISION_DETECTOR_H
