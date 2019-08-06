#ifndef COARSE_COLLISION_DETECTOR_H
#define COARSE_COLLISION_DETECTOR_H

#include <deque>
#include <vector>
#include "AABB.h"

namespace se::collision {

	class Collider;


	/**
	 * Class CoarseCollisionDetector, it's used to detect what colliders are
	 * intersecting by their AABBs
	 */
	class CoarseCollisionDetector
	{
	private:	// Nested types
		using ColliderPair = std::pair<const Collider*, const Collider*>;

		/** Holds cached data of a Collider */
		struct ColliderData
		{
			const Collider* collider;
			AABB aabb;
		};

	private:	// Attributes
		/** The Colliders to check if they collide between each other */
		std::deque<ColliderData> mColliders;

	public:		// Functions
		/** Adds the given Collider to the Detector so it will check if the
		 * Collider intersecs with the other Colliders in it.
		 *
		 * @param	collider a pointer to the Collider to submit */
		void submit(const Collider* collider);

		/** Calculates and returns the Colliders whose AABBs are currently
		 * intersecting
		 *
		 * @return	all the pairs of Colliders whose AABBs are intersecting
		 * @note	after calling this function all the Colliders will be
		 * 			cleared from the CoarseCollisionDetector */
		std::vector<ColliderPair> getIntersectingColliders();
	};

}

#endif		// COARSE_COLLISION_DETECTOR_H
