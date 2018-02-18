#ifndef EPA_COLLISION_DETECTOR_H
#define EPA_COLLISION_DETECTOR_H

#include <array>
#include <tuple>
#include <glm/glm.hpp>

namespace fe { namespace collision {

	class Contact;
	class ConvexCollider;
	struct Polytope;
	struct Triangle;


	/**
	 * Class EPACollisionDetector, it's the class used to calculate the
	 * Contact coordinates, normal and penetration from the given simplex
	 * polytopes with Expanding Polytope Algorithm.
	 */
	class EPACollisionDetector
	{
	private:	// Attributes
		/** The minimum difference between the distances to the origin of two
		 * faces needed for determinate the closest face to the origin */
		const float mMinFDifference;

		/** The precision of the projected point onto a triangle */
		const float mProjectionPrecision;

	public:		// Functions
		/** Creates a new EPACollisionDetector
		 * @param	minFDifference the minimum difference between two faces
		 *			needed for determinate the closest face in contact
		 * @param	projectionPrecision precision of the Contact coordinates */
		EPACollisionDetector(float minFDifference, float projectionPrecision) :
			mMinFDifference(minFDifference),
			mProjectionPrecision(projectionPrecision) {};

		/** Class destructor */
		~EPACollisionDetector() {};

		/** Calculates the deepest contact point between the given colliders
		 * using the EPA algorithm
		 *
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	polytope the convex shape to expand with EPA
		 * @return	the deepest Contact
		 * @note	Initially the polytope must hold a tetrahedron */
		Contact calculate(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Polytope& polytope
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
		 * @param	a tuple with a pointer to the closest face and its
		 *			distance to the origin */
		std::tuple<Triangle*, float> getClosestFaceToOrigin(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Polytope& polytope
		) const;

		/** Projects the given point onto the the given 3D triangle 
		 *
		 * @param	point the 3D coordinates of the point in world space
		 * @param	triangle an array with the 3 points of the triangle in
		 *			world space
		 * @param	projectedPoint a reference to the vector where we are going
		 *			to store the coordinates of the projected point in
		 *			barycentric coordinates
		 * @return	true if the point could be projected onto the triangle,
		 *			false otherwise*/
		bool projectPointOnTriangle(
			const glm::vec3& point,
			const std::array<glm::vec3, 3>& triangle,
			glm::vec3& projectedPoint
		) const;

		/** Returns the minimum distance to the origin of the given triangle
		 * @param	t the triangle to calculate its distance to the origin
		 * @return	the minimum distance of the triangle to the origin of
		 *			coordiantes */
		float getDistanceToOrigin(const Triangle& t) const;
	};

}}

#endif		// EPA_COLLISION_DETECTOR_H
