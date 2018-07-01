#ifndef EPA_COLLISION_DETECTOR_H
#define EPA_COLLISION_DETECTOR_H

#include <vector>

namespace fe { namespace collision {

	class Contact;
	class ConvexCollider;
	class Polytope;
	struct Edge;
	struct PolytopeFace;


	/**
	 * Class EPACollisionDetector, it's the class used to calculate the
	 * Contact coordinates, normal and penetration from the given simplex
	 * polytopes with Expanding Polytope Algorithm.
	 */
	class EPACollisionDetector
	{
	private:	// Attributes
		/** The maximum number of iterations of EPA Algorithm. If mMinFThreshold
		 * is too low this could make the algorithm exit prematurely */
		static const int sMaxIterations = 36;

		/** The minimum difference between the distances to the origin of
		 * a face and the next SupportPoint during the Polytope expansion step
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

		/** Class destructor */
		~EPACollisionDetector() {};

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
			std::vector<SupportPoint>& simplex, Contact& ret
		) const;
	private:
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

}}

#endif		// EPA_COLLISION_DETECTOR_H
