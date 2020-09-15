#ifndef COLLISION_WORLD_H
#define COLLISION_WORLD_H

#include <map>
#include <memory>
#include "Manifold.h"
#include "CoarseCollisionDetector.h"
#include "FineCollisionDetector.h"
#include "GJKRayCaster.h"
#include "../utils/MathUtils.h"
#include "../utils/PackedVector.h"

namespace se::collision {

	class Collider;


	/** Struct CollisionWorldData, holds all the Configuration parameters of the
	 * CollisionWorld */
	struct CollisionWorldData
	{
		/** The maximum number of collision Manifolds */
		std::size_t maxManifolds;

		/** The threshold value needed for checking if the closest face in
		 * contact was found */
		float minFDifference;

		/** The maximmum number of iterations of the collision detection
		 * algorithms */
		std::size_t maxIterations;

		/** The precision of the calculated Contact points */
		float contactPrecision;

		/** The minimum distance between the coordinates of two Contact used for
		 * used for checking if a contact is the same than another one */
		float contactSeparation;

		/** The maximum number of iterations for the ray casting algorithm */
		int maxRayCasterIterations;
	};


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
		using ManifoldUPtr = std::unique_ptr<Manifold>;
		using ManifoldCallback = std::function<void(const Manifold&)>;
		using RayCastCallback = std::function<
			void(const Collider&, const RayCast&)
		>;

	private:	// Attributes
		/** The CoarseCollisionDetector of the CollisionWorld. We will use
		 * it to check what Colliders are intersecting in the broad phase of
		 * the collision detection step */
		CoarseCollisionDetector mCoarseCollisionDetector;

		/** The FineCollisionDetector of the CollisionWorld. We will use
		 * it to generate all the contact data */
		FineCollisionDetector mFineCollisionDetector;

		/** The GJKRayCaster used for checking ray hits */
		GJKRayCaster mRayCaster;

		/** All the Manifolds that the CollisionWorld can hold */
		utils::PackedVector<Manifold> mManifolds;

		/** All the Colliders to check */
		std::vector<Collider*> mColliders;

		/** Maps a pair of Colliders with the index of the Manifold of their
		 * collision */
		std::unordered_map<ColliderPair, std::size_t, utils::PairHash>
			mCollidersManifoldMap;

	public:		// Functions
		/** Creates a new CollisionWorld
		 *
		 * @param	config the configuration parameters of the CollisionWorld */
		CollisionWorld(const CollisionWorldData& config);

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

		/** Calls the given callback for each of the active collision Manifolds
		 * generated during the last @see update call
		 *
		 * @param	callback the function to call */
		void processCollisionManifolds(const ManifoldCallback& callback) const;

		/** Checks which colliders intersects with the given ray and calls the
		 * given callback function for each of the intersecting colliders
		 *
		 * @param	rayOrigin the origin point of the ray
		 * @param	rayDirection the direction of the ray
		 * @param	callback the function to call */
		void processRayCast(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
			const RayCastCallback& callback
		) const;
	};

}

#endif		// COLLISION_WORLD_H
