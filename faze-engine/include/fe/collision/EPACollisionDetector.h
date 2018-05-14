#ifndef EPA_COLLISION_DETECTOR_H
#define EPA_COLLISION_DETECTOR_H

#include <vector>

namespace fe { namespace collision {

	class Contact;
	class ConvexCollider;
	struct Edge;
	struct Triangle;
	struct Polytope;


	/**
	 * Class EPACollisionDetector, it's the class used to calculate the
	 * Contact coordinates, normal and penetration from the given simplex
	 * polytopes with Expanding Polytope Algorithm.
	 */
	class EPACollisionDetector
	{
	private:	// Nested types
		typedef std::pair<std::vector<Triangle>::iterator, float> ClosestFace;

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
		 * @param	polytope the convex shape to expand with EPA
		 * @param	ret a reference to the deepest Contact (return parameter)
		 * @return	true if the Contact data was successfully created, false
		 *			otherwise */
		bool calculate(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Polytope& polytope, Contact& ret
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
		 * @param	polytope the Polytope to expand
		 * @param	a pair with a pointer to the closest face and its
		 *			distance to the origin */
		std::pair<Triangle*, float> calculateEPA(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Polytope& polytope
		) const;

		/** Calculates the closest face of the given polytope to the origin of
		 * the Minkowski Difference
		 *
		 * @param	polytope the Polytope with the faces
		 * @param	a pair with the iterator to the closest face in the polytope
		 *			and its distance to the origin */
		ClosestFace getClosestFaceToOrigin(Polytope& polytope) const;

		/** Appends the given edge to the edge vector if it isn't already
		 * inside, also if it founds an edge equal to the given one, it will
		 * remove it from the vector
		 *
		 * @param	e the edge to append
		 * @param	edgeVector the vector where we want to append the edge */
		void appendEdge(const Edge& e, std::vector<Edge>& edgeVector) const;
	};

}}

#endif		// EPA_COLLISION_DETECTOR_H
