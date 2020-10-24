#ifndef GJK_COLLISION_DETECTOR_H
#define GJK_COLLISION_DETECTOR_H

#include <utility>
#include <glm/glm.hpp>
#include "Simplex.h"

namespace se::collision {

	class ConvexCollider;


	/**
	 * Class GJKCollisionDetector, it's the class used for detecting collisions
	 * between arbitrary convex shapes with the GJK (Gilbert–Johnson–Keerthi)
	 * Algorithm.
	 *
	 * The GJK algorithm is an iterative algorithm that in each iteration tries
	 * to create a simplex from the points in the Configuration Space Object
	 * between the two convex shapes that contains the origin of coordinates.
	 *
	 * @note The algorithm implemented only works for 3 dimensions or less.
	 */
	class GJKCollisionDetector
	{
	private:	// Attributes
		/** The precision of the comparisons in the GJK algorithm */
		const float mEpsilon;

		/** The maximum number of iteration of the GJK algorithm */
		const std::size_t mMaxIterations;

	public:		// Functions
		/** Creates a new GJKCollisionDetector
		 *
		 * @param	epsilon the comparison precision of the algorithm
		 * @param	maxIterations the maximum number of iterations of the
		 *			algorithm */
		GJKCollisionDetector(float epsilon, std::size_t maxIterations) :
			mEpsilon(epsilon), mMaxIterations(maxIterations) {};

		/** Checks if the given ConvexColliders are intersecting with the GJK
		 * algorithm
		 *
		 * @param	collider1 the first ConvexCollider that we want to check
		 * @param	collider2 the second ConvexCollider that we want to check
		 * @return	a pair with the result of the GJK algorithm: a boolean that
		 *			tells if the two ConvexColliders are intersecting and a
		 *			vector with the SupportPoints of the simplex needed to
		 *			check the collision. */
		std::pair<bool, Simplex> calculateIntersection(
			const ConvexCollider& collider1, const ConvexCollider& collider2
		) const;
	private:
		/** Updates the given direction and simplex, reducing it to the lowest
		 * dimension possible by discarding vertices.
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The last point added should be
		 *			in the last position
		 * @param	searchDir the normalized direction used to search the next
		 *			SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex(Simplex& simplex, glm::vec3& direction) const;

		/** Updates the given direction and simplex in 0 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 0
		 * 			dimensions (one point)
		 * @param	searchDir the normalized direction used to search the next
		 *			SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex0D(Simplex& simplex, glm::vec3& direction) const;

		/** Updates the given direction and simplex in 1 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 1
		 * 			dimensions (a line) with the newest point in the last
		 * 			position
		 * @param	searchDir the normalized direction used to search the next
		 *			SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex1D(Simplex& simplex, glm::vec3& direction) const;

		/** Updates the given direction and simplex in 2 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 2
		 * 			dimensions (a triangle) with the newest point in the
		 * 			last position
		 * @param	searchDir the normalized direction used to search the next
		 *			SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex2D(Simplex& simplex, glm::vec3& direction) const;

		/** Updates the given direction and simplex in 3 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 3
		 * 			dimensions (a tetrahedron) with the newest point in
		 * 			the last position
		 * @param	searchDir the normalized direction used to search the next
		 *			SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex3D(Simplex& simplex, glm::vec3& direction) const;
	};

}

#endif		// GJK_COLLISION_DETECTOR_H
