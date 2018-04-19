#ifndef FINE_COLLISION_DETECTOR_H
#define FINE_COLLISION_DETECTOR_H

#include <vector>
#include <glm/glm.hpp>
#include "GJKCollisionDetector.h"
#include "EPACollisionDetector.h"

namespace fe { namespace collision {

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
	private:	// Attributes
		/** The minimum difference between the distances to the origin of two
		 * faces needed for determinate the closest face to the origin */
		static const float sMinFDifference;

		/** The precision of the calculated Contact points */
		static const float sContactPrecision;

		/** The minimum distance between the coordinates of two Contact used
		 * to determine if a contact is the same than other one */
		static const float sContactSeparation;

		/** The class that implements the GJK algorithm for detecting if two
		 * ConvexColliders are intersecting */
		GJKCollisionDetector mGJKCollisionDetector;

		/** The class that implements the EPA algorithm for calculating the
		 * Contact between the two ConvexColliders that are intersecting */
		EPACollisionDetector mEPACollisionDetector;

	public:		// Functions
		/** Creates a new FineCollisionDetector */
		FineCollisionDetector() :
			mEPACollisionDetector(sMinFDifference, sContactPrecision) {};

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
		);
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
		);

		/** Calculates the contact data of the collision that happened between
		 * a ConvexCollider and a ConcaveCollider
		 *
		 * @param	convexCollider the ConvexCollider with which we will
		 *			calculate the collision data
		 * @param	concaveCollider the ConcaveCollider with which we will
		 *			calculate the collision data
		 * @param	manifold a contact manifold where the FineCollisionDetector
		 *			will store the collision data
		 * @param	convexFirst if the ConvexCollider is the first collider in
		 *			the Manifold or the second one
		 * @return	true if the given Colliders are intersecting, false
		 *			otherwise */
		bool collideConvexConcave(
			const ConvexCollider& convexCollider,
			const ConcaveCollider& concaveCollider,
			Manifold& manifold, bool convexFirst
		);

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
		);

		/** Removes the Contacts that are no longer valid from the given
		 * manifold
		 *
		 * @param	manifold a reference to the Manifold whose contact we want
		 *			to remove */
		void removeInvalidContacts(Manifold& manifold) const;

		/** Checks if the given Contact is close to any of the older Contacts
		 *
		 * @param	newContact the Contact to compare
		 * @param	others the older contacts to compare
		 * @return	true if the newContact is close to any of the older Contacts,
		 *			false otherwise */
		bool isClose(
			const Contact& newContact,
			const std::vector<Contact>& others
		) const;

		/** Limts the number of contacts in the given manifold to 4, leaving
		 * inside the one with the deepest penetration and the 3 most separated
		 * between them
		 *
		 * @param	manifold a reference to the manifold
		 * @return	true if the number of contacs was cut down, false
		 *			otherwise */
		void limitManifoldContacts(Manifold& manifold) const;

		/** Calculates the minimum distance from a point to the given edge
		 *
		 * @param	p the point whose distance to the edge we want to know
		 * @param	e1 the first point of the edge
		 * @param	e2 the second point of the edge
		 * @return	the minimum distance of p to the edge between e1 and e2 */
		float distancePointEdge(
			const glm::vec3& p,
			const glm::vec3& e1, const glm::vec3& e2
		) const;

		/** Calculates the minimum distance from a point to the given triangle
		 *
		 * @param	p the point whose distance to the triangle we want to know
		 * @param	t1 the first point of the triangle
		 * @param	t2 the second point of the triangle
		 * @param	t3 the third point of the triangle
		 * @return	the minimum distance of p to the triangle between e1, e2
		 *			and e3 */
		float distancePointTriangle(
			const glm::vec3& p,
			const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3
		) const;
	};

}}

#endif		// FINE_COLLISION_DETECTOR_H
