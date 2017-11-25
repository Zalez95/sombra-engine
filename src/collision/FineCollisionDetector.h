#ifndef FINE_COLLISION_DETECTOR_H
#define FINE_COLLISION_DETECTOR_H

#include <vector>
#include <glm/glm.hpp>

namespace collision {

	class Contact;
	class Manifold;
	class Collider;
	class ConvexCollider;
	class ConcaveCollider;


	/**
	 * Class FineCollisionDetector, is the class that calculates the data
	 * generated from the intersection (collision) of volumes (Colliders)
	 */
	class FineCollisionDetector
	{
	private:	// Nested types
		struct SupportPoint;
		struct Edge;
		struct Triangle;
		static const float CONTACT_TOLERANCE;
		static const float CONTACT_SEPARATION;

	public:		// Functions
		/** Creates a new FineCollisionDetector */
		FineCollisionDetector() {};

		/** Class destructor */
		~FineCollisionDetector() {};

		/** Calculates the contact data of the collision that happened between
		 * the given Colliders
		 * 
		 * @param	collider1 a pointer to the first Collider with which we
		 *			will calculate the collision data
		 * @param	collider2 a pointer to the second Collider with which we
		 *			will calculate the collision data
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collide(
			const Collider* collider1, const Collider* collider2,
			Manifold& manifold
		) const;
	private:
		/** Calculates the contact data of the collision that happened between
		 * the given ConcaveColliders
		 * 
		 * @param	collider1 the first of the ConcaveColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConcaveColliders that are
		 *			intersecting
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collideConcave(
			const ConcaveCollider& collider1,
			const ConcaveCollider& collider2,
			Manifold& manifold
		) const;

		/** Calculates the contact data of the collision that happened between
		 * a ConvexCollider and a ConcaveCollider
		 * 
		 * @param	convexCollider the ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	concaveCollider the ConcaveCollider with which we will
		 *			calculate the collision data
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collideConvexConcave(
			const ConvexCollider& convexCollider,
			const ConcaveCollider& concaveCollider,
			Manifold& manifold
		) const;

		/** Calculates the contact data of the collision that happened between
		 * the given ConvexColliders
		 * 
		 * @param	collider1 the first ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	collider2 the second ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collideConvex(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			Manifold& manifold
		) const;

		/** Calculates if the given ConvexColliders are intersecting or not
		 * with the GJK algorithm
		 *
		 * @param	collider1 the first ConvexColliders that we want to check
		 * @param	collider2 the second ConvexColliders that we want to check
		 * @param	simplex the simplex needed to check the collision.
		 * 			If the origin is inside the simplex the two meshes are
		 *			intersecting
		 * @return	true if the two Colliders collides, false otherwise */
		bool calculateGJK(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<SupportPoint>& simplex
		) const;

		/** Creates an initial polytope (tetrahedron) from the given simplex
		 * 
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	simplex the simplex needed to create the tetrahedron.
		 *			Initially it could hold a segment, a triangle or a
		 *			tetrahedron
		 * @return	the tetrahedron polytope
		 * @note	the simplex initially must hold inside at least one
		 *			SupportPoint */
		std::vector<Triangle> createPolytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<SupportPoint>& simplex
		) const;

		/** Calculates the deepest contact point between the given colliders
		 * using the EPA algorithm
		 *
		 * @param	collider1 the first of the ConvexColliders that are
		 *			intersecting
		 * @param	collider2 the second of the ConvexColliders that are
		 *			intersecting
		 * @param	polytope the convex shape to expand with EPA
		 * @return	the deepest contact point
		 * @note	Initially the polytope must hold a tetrahedron */
		Contact calculateEPA(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<Triangle>& polytope
		) const;

		/** Removes the Contacts that are no longer valid from the given
		 * manifold
		 * 
		 * @param	manifold a reference to the Manifold whose contact we want
		 *			to remove */
		void removeInvalidContacts(Manifold& manifold) const;

		/** Checks if the given Contact is close to any of the Contacts in the
		 * given Manifold
		 * 
		 * @param	newContact the Contact to compare
		 * @param	manifold the Manifold that holds the Contacts
		 * @return	true if the newContact is close to any of the Contacts in
		 *			the Manifold, false otherwise */
		bool isClose(const Contact& newContact, const Manifold& others) const;

		/** Limts the number of contacts in the given manifold to 4, leaving
		 * inside the one with the deepest penetration and the 3 most separated
		 * between them
		 * 
		 * @param	manifold a reference to the manifold
		 * @return	true if the number of contacs was cut down, false
		 *			otherwise */
		void limitManifoldContacts(Manifold& manifold) const;

		/** Calculates the furthest SupportPoint in the given direction inside
		 * the Minkowski Difference (or the Configuration Space Object) of the
		 * given colliders
		 * 
		 * @param	collider1 the first convex ConvexCollider with which we
		 *			want to calculate the support point
		 * @param	collider2 the second convex ConvexCollider with which we
		 *			want to calculate the support point
		 * @param	searchDir the direction to search the SupportPoint
		 * @return	a point in the Minkowski difference of the two
		 *			ConvexColliders */
		SupportPoint getSupportPoint(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			const glm::vec3& searchDir
		) const;

		/** Updates the given direction and simplex, reducing it to the lowest
		 * dimension possible by discarding vertices.
		 *
		 * @param	simplex a vector with the coordinates in world space of
		 * 			the points of the simplex. The latest point added should
		 * 			be in the last position
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

}

#endif		// FINE_COLLISION_DETECTOR_H
