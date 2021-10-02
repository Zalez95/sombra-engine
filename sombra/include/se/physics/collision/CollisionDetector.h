#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <map>
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"
#include "../../utils/MathUtils.h"
#include "../../utils/PackedVector.h"

namespace se::physics {

	class RigidBody;
	class RigidBodyWorld;


	/**
	 * Class CollisionListener, class used for being notified of the collisions
	 * detected by the CollisionDetector
	 */
	class ICollisionListener
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ICollisionListener() = default;

		/** Function called per collision Manifold updated
		 *
		 * @param	manifold the Manifold updated */
		virtual void onCollision(const Manifold& manidold) = 0;
	};


	/**
	 * Class CollisionDetector, it's a class used for detecting collisions
	 * between Colliders. It will also calculate the Manifolds and its Contact
	 * data.
	 *
	 * The collision detection it splitted in two phases, one coarse collision
	 * detection where we're going to check which colliders could be
	 * intersecting by its AABB intersections, and one fine collision detection
	 * where we will calculate the Contact data of the collisions.
	 */
	class CollisionDetector
	{
	private:	// Nested types
		using ColliderPair = std::pair<const Collider*, const Collider*>;
		using ManifoldUPtr = std::unique_ptr<Manifold>;
		using ManifoldCallback = std::function<void(const Manifold&)>;

	private:	// Attributes
		/** A reference to the RigidBodyWorld that holds the RigidBodies */
		RigidBodyWorld& mParentWorld;

		/** The CoarseCollisionDetector of the CollisionDetector. We will use
		 * it to check what Colliders are intersecting in the broad phase of
		 * the collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the CollisionDetector. We will use
		 * it to generate all the contact data */
		FineCollisionDetector mFineCollisionDetector;

		/** All the Manifolds that the CollisionDetector can hold */
		utils::PackedVector<Manifold> mManifolds;

		/** Maps a pair of Colliders with the index of the Manifold of their
		 * collision */
		std::unordered_map<ColliderPair, std::size_t, utils::PairHash>
			mCollidersManifoldMap;

		/** The listeners added to the CollisionDetector */
		std::vector<ICollisionListener*> mListeners;

	public:		// Functions
		/** Creates a new CollisionDetector
		 *
		 * @param	parentWorld the RigidBodyWorld that holds all the
		 *			RigidBodies to update */
		CollisionDetector(RigidBodyWorld& parentWorld);

		/** Calculates all the collisions that are currently happening between
		 * the RigidBodies */
		void update();

		/** Removes all the Manifolds that references the given RigidBody
		 *
		 * @param	rigidBody a pointer to the RigidBody to remove
		 * @note	the removed Manifolds won't be notified */
		void removeRigidBody(const RigidBody* rigidBody);

		/** Adds the given ICollisionListener to the CollisionDetector so it
		 * will be updated of the collisions
		 *
		 * @param	listener a pointer to the new ICollisionListener */
		void addListener(ICollisionListener* listener);

		/** Removes the given ICollisionListener from the CollisionDetector so
		 * it won't longer be updated of the collisions
		 *
		 * @param	listener a pointer to the ICollisionListener to remove */
		void removeListener(ICollisionListener* listener);
	};

}

#endif		// COLLISION_DETECTOR_H
