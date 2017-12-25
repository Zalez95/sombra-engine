#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <set>
#include <map>
#include <memory>
#include "Manifold.h"

namespace fe { namespace collision {

	class Collider;

	/**
	 * Class CollisionDetector
	 */
	class CollisionDetector
	{
	private:	// Nested types
		typedef std::pair<const Collider*, const Collider*> ColliderPair;
		struct Implementation;

	private:	// Attributes
		/** All the Colliders to check */
		std::set<const Collider*> mColliders;

		/** Maps a pair of Colliders with the the Manifold of it's collision */
		std::map<ColliderPair, Manifold> mMapCollidersManifolds;

		/** The cached pointers to all the current contact Manifolds of all
		 * the detected collisions */
		std::set<Manifold*> mManifolds;

		/** Other attributes hided with the Pimpl idiom */
		std::unique_ptr<Implementation> mImpl;

	public:		// Functions
		/** Creates a new CollisionDetector */
		CollisionDetector();

		/** Class destructor */
		~CollisionDetector();

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
