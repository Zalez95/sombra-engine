#ifndef FINE_COLLISION_DETECTOR_H
#define FINE_COLLISION_DETECTOR_H

#include <vector>
#include <glm/glm.hpp>

namespace collision {

	class Contact;
	class Collider;
	class BoundingSphere;
	class ConvexPolyhedron;


	/**
	 * Class FineCollisionDetector, is the class that calculates the data
	 * generated from the intersection (collision) of volumes (Colliders)
	 */
	class FineCollisionDetector
	{
	private:	// Nested types
		struct SupportPoint;
		struct Triangle;
		struct Edge;
		static const float TOLERANCE;

	public:		// Functions
		/** Creates a new FineCollisionDetector */
		FineCollisionDetector() {};

		/** Class destructor */
		~FineCollisionDetector() {};

		/** Returns the data of the collision that happened between the given
		 * Colliders
		 * 
		 * @param	collider1 the first Collider with which we will calculate
		 *			the collision data
		 * @param	collider2 the second Collider with which we will calculate
		 *			the collision data
		 * @return	the data of the Collision */
		std::vector<Contact> collide(
			const Collider& collider1,
			const Collider& collider2
		) const;
	private:
		/** Calculates if the two Colliders are intersecting with the GJK
		 * algorithm
		 *
		 * @param	collider1 the first convex Colliders that we want to check
		 * @param	collider2 the second convex Colliders that we want to check
		 * @param	simplex the simplex needed to check the collision.
		 * 			If the origin is inside the simplex in 3D (tetrahedron)
		 * 			the two meshes are intersecting
		 * @return	true if the two meshes collides, false otherwise */
		bool calculateGJK(
			const Collider& collider1,
			const Collider& collider2,
			std::vector<SupportPoint>& simplex
		) const;

		/** Calculates the contact points using the EPA algorithm
		 *
		 * @param	collider1 the first of the convex Colliders that collides
		 * @param	collider2 the second of the convex Colliders that collides
		 * @param	polytope the convex shape to expand with EPA. Initially it
		 *			must hold a tetrahedron.
		 * @return	the contact point in world space */
		Contact calculateEPA(
			const Collider& collider1, const Collider& collider2,
			std::vector<Triangle>& polytope
		) const;

		/** Calculates the furthest SupportPoint in the given direction inside
		 * the Minkowski Difference (or the Configuration Space Object) of the
		 * given colliders
		 * 
		 * @param	collider1 the first convex Collider with which we want to
		 *			calculate the support point
		 * @param	collider2 the second convex Collider with which we want to
		 *			calculate the support point
		 * @param	direction a vector pointing to the direction
		 * @return	a point in the Minkowski difference of the two Colliders */
		SupportPoint getSupportPoint(
			const Collider& collider1,
			const Collider& collider2,
			const glm::vec3& direction
		) const;

		/** Updates the given direction and simplex and returns true if the
		 * origin is inside the simplex 3D (tetrahedron), false otherwise
		 *
		 * @return	true if the origin is inside the given simplex, false
		 * 			otherwise
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The latest point added should
		 * 			be in the last position
		 * @param	direction the direction to where the origin is located in
		 * 			relation to the current simplex */
		bool doSimplex(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 0 dimensions
		 *
		 * @return	false always
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 0
		 * 			dimensions (one point)
		 * @param	direction the direction to where the origin is located in
		 * 			relation to the current simplex */
		bool doSimplex0D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 1 dimensions
		 *
		 * @return	false always
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 1
		 * 			dimensions (a line) with the last added point in the last
		 * 			position
		 * @param	direction the direction to where the origin is located in
		 * 			relation to the current simplex */
		bool doSimplex1D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 2 dimensions
		 *
		 * @return	false always
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 2
		 * 			dimensions (a triangle) with the last added point in the
		 * 			last position
		 * @param	direction the direction to where the origin is located in
		 * 			relation to the current simplex */
		bool doSimplex2D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;

		/** Updates the given direction and simplex in 3 dimensions
		 *
		 * @return	false always
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The simplex must have 3
		 * 			dimensions (a tetrahedron) with the last added point in
		 * 			the last position
		 * @param	direction the direction to where the origin is located in
		 * 			relation to the current simplex */
		bool doSimplex3D(
			std::vector<SupportPoint>& simplex, glm::vec3& direction
		) const;
	};

}

#endif		// FINE_COLLISION_DETECTOR_H
