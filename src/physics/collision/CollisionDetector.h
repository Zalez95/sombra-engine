#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <vector>
#include <glm/glm.hpp>
#include "Contact.h"

namespace physics {

	class Collider;
	struct AABB;
	class Plane;
	class BoundingSphere;
	class ConvexPolyhedron;


	/**
	 * Class CollisionDetector, is the class that calculates the data
	 * generated from the intersection (collision) of volumes (Colliders)
	 */
	class CollisionDetector
	{
	private:	// Nested types
		struct SupportPoint;
		struct Triangle;
		struct Edge;

		static const float TOLERANCE;

	public:		// Functions
		/** Creates a new CollisionDetector */
		CollisionDetector() {};

		/** Class destructor */
		~CollisionDetector() {};

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
		/** Board phase of the collision detection process.
		 * 
		 * @param	collider1 the first Collider that we want to check if
		 *			it's intersecting
		 * @param	collider2 the second Collider that we want to check if
		 *			it's intersecting
		 * @return	true if both colliders are intersecting between them,
		 *			false otherwise */
		bool coarseCollisionDetection(
			const Collider& collider1,
			const Collider& collider2
		) const;

		/** Calculates and returns the contact points of the collision that
		 * happened between the given Colliders
		 *
		 * @param	collider1 the first Collider with which we will calculate
		 *			the collision data
		 * @param	collider2 the second Collider with which we will calculate
		 *			the collision data
		 * @return	the contact points of the Collision */
		std::vector<Contact> fineCollisionDetection(
			const Collider& collider1,
			const Collider& collider2
		) const;

		/** Calculates and returns the contact points of the collision that
		 * happened between the given BoundingSpheres
		 *  
		 * @param	sphere1 the first BoundingSphere with which we will
		 *			calculate the data of the collision
		 * @param	sphere2 the second BoundingSphere with which we will
		 *			calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSpheres(
			const BoundingSphere& sphere1,
			const BoundingSphere& sphere2
		) const;

		/** Calculates and returns the contact points of the collision that
		 * happened between the given BoundingSphere and the given Plane
		 * 
		 * @note	the BoundingSphere can collide with the plane only if it
		 *			crosses the plane in the opposite direction of the
		 *			plane's normal
		 * @param	sphere the BoundingSphere with which we will calculate
		 *			the data of the collision
		 * @param	plane the Plane with which we will calculate the data
		 *			of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSphereAndPlane(
			const BoundingSphere& sphere,
			const Plane& plane
		) const;

		/** Calculates and returns the contact points of the collision that
		 * happened between the given ConvexPolyhedrones
		 *
		 * @param	cp1 the first ConvexPolyhedron with which we will
		 * 			calculate the data of the collision
		 * @param	cp2 the second ConvexPolyhedron with which we will
		 * 			calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideConvexPolys(
			const ConvexPolyhedron& cp1,
			const ConvexPolyhedron& cp2
		) const;
		
		/** Calculates and returns the contact points of the collision that
		 * happened between the given BoudingBox and the given Plane
		 * 
		 * @param	cp the ConvexPolyhedron with which we will calculate
		 * 			the data of the collision
		 * @param	plane the Plane with which we will calculate the data
		 *			of the collision
		 * @return	the data of the collision
		 * @note	the ConvexPolyhedron can collide with the plane only if it
		 * 			crosses the plane in the opposite direction of the plane's
		 * 			normal */
		std::vector<Contact> collideConvexPolyAndPlane(
			const ConvexPolyhedron& cp,
			const Plane& plane
		) const;

		/** Calculates and returns the contact points of the collision that
		 * happened between the given ConvexPolyhedron and the given
		 * BoundingSphere
		 *  
		 * @param	cp the ConvexPolyhedron with which we will calculate
		 *			the data of the collision
		 * @param	sphere the BoundingSphere with which we will calculate
		 *			the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideConvexPolyAndSphere(
			const ConvexPolyhedron& cp,
			const BoundingSphere& sphere
		) const;
		
		/** Calculates if the two meshes are intersecting with the GJK
		 * algorithm
		 *
		 * @param	mesh1 the first convex mesh that we want to check
		 * @param	mesh2 the second convex mesh that we want to check
		 * @param	simplex the simplex needed to check the collision.
		 * 			If the origin is inside the simplex in 3D (tetrahedron)
		 * 			the two meshes are intersecting
		 * @return	true if the two meshes collides, false otherwise */
		bool calculateGJK(
			const std::vector<glm::vec3>& mesh1,
			const std::vector<glm::vec3>& mesh2,
			std::vector<SupportPoint>& simplex
		) const;

		/** Calculates the closest Triangle, its normal and distance to the
		 * origin from the given data using the EPA algorithm
		 *
		 * @param	mesh1 the first of the convex meshes that collides
		 * @param	mesh2 the second of the convex meshes that collides
		 * @param	polytope the convex shape to expand with EPA
		 * @param	closestFace the Triangle where we will store the closest
		 *			face
		 * @param	closestFaceDist the float where we will store the distance
		 * 			of the closest face to the origin */
		void calculateEPA(
			const std::vector<glm::vec3>& mesh1,
			const std::vector<glm::vec3>& mesh2,
			std::vector<Triangle>& polytope,
			Triangle& closestFace, float& closestFaceDist
		) const;

		/** @return	a point in the Minkowski difference of the two meshes
		 * @param	mesh1 the vertices of the first mesh with which we want to
		 *			calculate the support point
		 * @param	mesh2 the vertices of the second mesh with which we want to
		 *			calculate the support point
		 * @param	direction a vector pointing to the direction */
		SupportPoint getSupportPoint(
			const std::vector<glm::vec3>& mesh1,
			const std::vector<glm::vec3>& mesh2,
			const glm::vec3& direction
		) const;

		/** @return	the coordinates in world space of furthest point of the
		 * 			given mesh in the given direction
		 * @param	mesh the vertices of the mesh in world space
		 * @param	direction the direction to where we want to get the
		 * 			furthest point */
		glm::vec3 getFurthestPointInDirection(
			const std::vector<glm::vec3>& mesh,
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

#endif		// COLLISION_DETECTOR_H
