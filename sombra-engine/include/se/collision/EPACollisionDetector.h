#ifndef EPA_COLLISION_DETECTOR_H
#define EPA_COLLISION_DETECTOR_H

#include "../utils/FixedVector.h"

namespace se::collision {

	struct Contact;
	class ConvexCollider;
	class Polytope;


	/**
	 * Class EPACollisionDetector, it's the class used to calculate the
	 * Contact coordinates, normal and penetration from the given simplex
	 * polytopes with Expanding Polytope Algorithm.
	 */
	class EPACollisionDetector
	{
	private:	// Nested types
		using InitialSimplex = utils::FixedVector<SupportPoint, 4>;

	private:	// Attributes
		/** The minimum difference between the distances to the origin of
		 * a HEFace and the next SupportPoint during the Polytope expansion step
		 * needed for determinate the closest face to the origin */
		const float mMinFThreshold;

		/** The precision of the projected point onto a triangle */
		const float mProjectionPrecision;

	public:		// Functions
		/** Creates a new EPACollisionDetector
		 * @param	minFThreshold a threshold needed for determinate the
		 *			closest face in contact and stop the Algorithm
		 * @param	projectionPrecision precision of the Contact coordinates */
		EPACollisionDetector(float minFThreshold, float projectionPrecision);

		/** Calculates the deepest Contact point between the given colliders
		 * using the EPA algorithm
		 *
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	simplex the points of the initial simplex
		 * @param	ret a reference to the deepest Contact (return parameter)
		 * @return	true if the Contact data was successfully created, false
		 *			otherwise */
		bool calculate(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			InitialSimplex& simplex, Contact& ret
		) const;
	private:
		/** Creates an initial polytope from the given simplex
		 *
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	simplex the points of the initial simplex
		 * @return	the polytope
		 * @note	if the given simplex is an edge or a triangle it will be
		 *			expanded to a tetrahedron */
		Polytope createInitialPolytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			InitialSimplex& simplex
		) const;

		/** Expands the given edge simplex to a tetrahedron
		 *
		 * @param	collider1 the first of the colliders used to create the
		 *			simplex
		 * @param	collider2 the second of the colliders used to create the
		 *			simplex
		 * @param	the initial simplex used to create the polytope. In this
		 *			case the simplex must be an edge (size == 2) */
		void tetrahedronFromEdge(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			InitialSimplex& simplex
		) const;

		/** Expands the given triangle simplex to a tetrahedron
		 *
		 * @param	collider1 the first of the colliders used to create the
		 *			simplex
		 * @param	collider2 the second of the colliders used to create the
		 *			simplex
		 * @param	the initial simplex used to create the polytope. In this
		 *			case the simplex must be a triangle (size == 3) */
		void tetrahedronFromTriangle(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			InitialSimplex& simplex
		) const;

		/** Checks if the given tetrahedron has the origin of coordinates inside
		 *
		 * @param	vertices the 4 vertices of the tetrahedron
		 * @return	true if the origin of coordinates is inside the tetrahedron,
		 *			false otherwise */
		bool isOriginInsideTetrahedron(
			const utils::FixedVector<glm::vec3, 4>& vertices
		) const;

		/** Expands the given polytope iteratively until it finds the closest
		 * face to the origin with the EPA Algorithm (in other words, when a
		 * face that it's closer to the origin than the old closest face with
		 * a difference of sMinFDifference)
		 *
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	polytope a reference to the Polytope to expand its faces
		 * @return	the index of the closest face if the polytope was expanded
		 *			successfully, -1 otherwise
		 * @note	the initial polytope must be a tetrahedron */
		int expandPolytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Polytope& polytope
		) const;

		/** Fills the Contact data with the normal of closest face in the
		 * Polytope to the origin, and the distance and coordinates of the
		 * closest point of this face to the origin
		 *
		 * @param	polytope the Polytope with which we want to fill the Contact
		 *			data
		 * @param	iClosestFace the index of the closest face in the Polytope
		 * @param	ret a reference to the Contact where we are going to store
		 *			the data */
		void fillContactData(
			const Polytope& polytope, int iClosestFace, Contact& ret
		) const;
	};

}

#endif		// EPA_COLLISION_DETECTOR_H
