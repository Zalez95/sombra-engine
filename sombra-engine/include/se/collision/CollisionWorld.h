#ifndef COLLISION_WORLD_H
#define COLLISION_WORLD_H

#include <map>
#include "Manifold.h"
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"

namespace se::collision {

	class Collider;


	/**
	 * Class CollisionWorld, it's a class used to group colliders and to detect
	 * collisions between them. It will also calculate the Manifolds and its
	 * Contact data.
	 *
	 * The collision detection it splitted in two phases, one coarse collision
	 * detection where we're going to check which colliders could be
	 * intersecting by its AABB intersections, and one fine collision detection
	 * where we will calculate the Contact data of the collisions.
	 */
	class CollisionWorld
	{
	private:	// Nested types
		using ColliderPair = std::pair<const Collider*, const Collider*>;

	private:	// Attributes
		static constexpr float kMinFDifference		= 0.00001f;
		static constexpr float kContactPrecision	= 0.0000001f;
		static constexpr float kContactSeparation	= 0.00001f;

		/** The CoarseCollisionDetector of the CollisionWorld. We will use
		 * it to check what Colliders are intersecting in the broad phase of
		 * the collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the CollisionWorld. We will use
		 * it to generate all the contact data */
		FineCollisionDetector mFineCollisionDetector;

		/** All the Colliders to check */
		std::vector<Collider*> mColliders;

		/** Maps a pair of Colliders with the the Manifold of it's collision */
		std::map<ColliderPair, Manifold> mMapCollidersManifolds;

		/** The cached pointers to all the current contact Manifolds of all
		 * the detected collisions */
		std::vector<const Manifold*> mManifolds;

	public:		// Functions
		/** Creates a new CollisionWorld */
		CollisionWorld();

		/** @return	all the contact Manifolds of the detected collisions */
		const std::vector<const Manifold*>& getCollisionManifolds() const
		{ return mManifolds; };

		/** Adds the given Collider to the CollisionWorld so it will
		 * check if it collides with the other Colliders
		 *
		 * @param	collider a pointer to the Collider that we want to add */
		void addCollider(Collider* collider);

		/** Removes the given Collider from the CollisionWorld so it wont't
		 * check if it collides with the other Colliders
		 *
		 * @param	collider a pointer to the Collider that we want to
		 * 			remove */
		void removeCollider(Collider* collider);

		/** Calculates all the collisions that are currently happening between
		 * the Colliders added to the CollisionWorld
		 *
		 * @note	the new intersecting Manifolds will be added to the Manifold
		 *			vector returned by @see getCollisionManifolds, while the
		 *			old non intersecting ones will be removed from it */
		void update();
	};

}

#endif		// COLLISION_WORLD_H
