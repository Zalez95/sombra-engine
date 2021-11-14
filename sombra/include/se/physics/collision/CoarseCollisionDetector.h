#ifndef COARSE_COLLISION_DETECTOR_H
#define COARSE_COLLISION_DETECTOR_H

#include <memory>
#include <functional>
#include "../../utils/PackedVector.h"
#include "Ray.h"

namespace se::physics {

	class Collider;
	template <typename T> class AABBAVLTree;


	/**
	 * Class CoarseCollisionDetector, it's used to detect which colliders are
	 * intersecting by their AABBs using an AABB Tree
	 */
	class CoarseCollisionDetector
	{
	public:		// Nested types
		using CollisionCallback = std::function<void(Collider*, Collider*)>;
		using ColliderCallback = std::function<void(Collider*)>;
	private:
		/** Holds cached data of a Collider */
		struct ColliderData
		{
			Collider* collider;
			std::size_t nodeId;
		};

	public:	// Attributes
		/** The epsilon value used for the comparisons */
		float mEpsilon;

		/** The Colliders to check if they collide between each other */
		utils::PackedVector<ColliderData> mColliders;

		/** The AABB Tree used for the coarse collision detection, the user
		 * data is an index to a Collider in @see mColliders */
		std::unique_ptr<AABBAVLTree<std::size_t>> mAABBTree;

	public:	// Functions
		/** Creates a new CoarseCollisionDetector
		 *
		 * @param	epsilon the Epsilon value used for the tests */
		CoarseCollisionDetector(float epsilon);

		/** Class destructor */
		~CoarseCollisionDetector();

		/** Adds the given Collider to the Detector so it will check if the
		 * Collider intersecs with the other Colliders in it
		 *
		 * @param	collider a pointer to the Collider to add */
		void add(Collider* collider);

		/** Iterates through all the Colliders added to the Detector
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each Collider */
		void processColliders(const ColliderCallback& callback) const;

		/** Removes the given Collider from the Detector so it won't longer
		 * check if the Collider intersecs with the other Colliders in it
		 *
		 * @param	collider a pointer to the Collider to remove */
		void remove(Collider* collider);

		/** Updates the Detector with the movement of the Colliders, this must
		 * be called at every clock tick */
		void update();

		/** Calculates all the Colliders that are currently intersecting
		 *
		 * @param	callback the function that must be called for
		 *			every pair of Colliders intersecting */
		void calculateCollisions(const CollisionCallback& callback) const;

		/** Calculates all the Colliders that are currently intersecting with
		 * the given ray
		 *
		 * @param	ray the ray to test
		 * @param	callback the function that must be called for each
		 *			of the Colliders intersecting with the ray */
		void calculateIntersections(
			const Ray& ray, const ColliderCallback& callback
		) const;
	};

}

#endif		// COARSE_COLLISION_DETECTOR_H
