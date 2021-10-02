#ifndef COARSE_COLLISION_DETECTOR_H
#define COARSE_COLLISION_DETECTOR_H

#include <deque>
#include <functional>
#include "AABB.h"

namespace se::physics {

	class Collider;


	/**
	 * Class CoarseCollisionDetector, it's used to detect what colliders are
	 * intersecting by their AABBs
	 */
	class CoarseCollisionDetector
	{
	private:	// Nested types
		using ColliderPair = std::pair<const Collider*, const Collider*>;
		using IntersectionCallback = std::function<void(const ColliderPair&)>;

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

		/** Calls the given callback for each of the detected intersections
		 * between Colliders AABBs
		 *
		 * @param	callback the function to call
		 * @note	after calling this function all the Colliders will be
		 * 			cleared from the CoarseCollisionDetector */
		void processIntersectingColliders(const IntersectionCallback& callback);
	};

}

#endif		// COARSE_COLLISION_DETECTOR_H
