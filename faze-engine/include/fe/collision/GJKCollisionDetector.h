#ifndef GJK_COLLISION_DETECTOR_H
#define GJK_COLLISION_DETECTOR_H

#include <vector>
#include <glm/glm.hpp>
#include "SupportPoint.h"

namespace fe { namespace collision {

	class ConvexCollider;


	/**
	 * Class GJKCollisionDetector, it's the class used to detect collisions
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
		/** A vector with the SupportPoints of the simplex needed to check
		 * the collision. If the origin is inside the simplex then the two
		 * colliders are intersecting */
		std::vector<SupportPoint> mSimplex;

	public:		// Functions
		/** Creates a new GJKCollisionDetector */
		GJKCollisionDetector() {};

		/** Class destructor */
		~GJKCollisionDetector() {};

		/** Calculates if the given ConvexColliders are intersecting or not
		 * with the GJK algorithm
		 *
		 * @param	collider1 the first ConvexColliders that we want to check
		 * @param	collider2 the second ConvexColliders that we want to check
		 * @return	true if the two Colliders collides, false otherwise */
		bool calculate(
			const ConvexCollider& collider1, const ConvexCollider& collider2
		);

		/** @return	a vector with the SupportPoints of the simplex needed to
		 *			check the collision. */
		inline std::vector<SupportPoint> getLastSimplex() const
		{ return mSimplex; };
	private:
		/** Updates the given direction and simplex, reducing it to the lowest
		 * dimension possible by discarding vertices.
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The last point added should be
		 *			in the last position
		 * @param	searchDir the direction to search the next SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 0 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 0
		 * 			dimensions (one point)
		 * @param	searchDir the direction to search the next SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex0D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 1 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 1
		 * 			dimensions (a line) with the newest point in the last
		 * 			position
		 * @param	searchDir the direction to search the next SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex1D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 2 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 2
		 * 			dimensions (a triangle) with the newest point in the
		 * 			last position
		 * @param	searchDir the direction to search the next SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex2D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 3 dimensions
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 3
		 * 			dimensions (a tetrahedron) with the newest point in
		 * 			the last position
		 * @param	searchDir the direction to search the next SupportPoint
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise */
		bool doSimplex3D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;
	};

}}

#endif		// GJK_COLLISION_DETECTOR_H
