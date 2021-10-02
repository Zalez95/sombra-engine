#ifndef EPA_COLLISION_DETECTOR_H
#define EPA_COLLISION_DETECTOR_H

#include "Simplex.h"
#include "Polytope.h"

namespace se::physics {

	struct Contact;
	class ConvexCollider;


	/**
	 * Class EPACollisionDetector, it's the class used to calculate the
	 * Contact coordinates, normal and penetration from the given simplex
	 * polytopes with Expanding Polytope Algorithm.
	 */
	class EPACollisionDetector
	{
	private:	// Attributes
		/** The minimum difference between the distances to the origin of
		 * a HEFace and the next SupportPoint during the Polytope expansion step
		 * needed for checking if we found the closest face to the origin */
		const float mMinFThreshold;

		/** The maximum number of iteration of the GJK algorithm */
		const std::size_t mMaxIterations;

		/** The precision of the projected point onto a triangle */
		const float mProjectionPrecision;

		/** The vectors needed for expanding the polytope */
		std::vector<int> mOverlappingFaces, mHorizon, mFacesToRemove;

	public:		// Functions
		/** Creates a new EPACollisionDetector
		 * @param	minFThreshold a threshold value needed for checking if we
		 *			found the closest face in contact and stop the Algorithm
		 * @param	maxIterations the maximum number of iterations of the
		 *			algorithm
		 * @param	projectionPrecision precision of the Contact coordinates */
		EPACollisionDetector(
			float minFThreshold, std::size_t maxIterations,
			float projectionPrecision
		);

		/** Calculates the deepest Contact point between the given colliders
		 * using the EPA algorithm
		 *
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	simplex the points of the initial simplex
		 * @return	a pair with a boolean that tells if the Contact data was
		 *			successfully created and the deepest Contact */
		std::pair<bool, Contact> calculate(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Simplex& simplex
		);
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
			Simplex& simplex
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
			Simplex& simplex
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
			Simplex& simplex
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
		);

		/** Calculates the Contact data with the normal of closest face in the
		 * Polytope to the origin, and the distance and coordinates of the
		 * closest point of this face to the origin
		 *
		 * @param	polytope the Polytope with which we want to fill the Contact
		 *			data
		 * @param	iClosestFace the index of the closest face in the Polytope
		 * @return	the Contact data */
		Contact calculateContactData(
			const Polytope& polytope, int iClosestFace
		) const;
	};

}

#endif		// EPA_COLLISION_DETECTOR_H
