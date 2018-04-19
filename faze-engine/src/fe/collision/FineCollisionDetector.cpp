#include <tuple>
#include <algorithm>
#include "fe/collision/Contact.h"
#include "fe/collision/Manifold.h"
#include "fe/collision/Polytope.h"
#include "fe/collision/Collider.h"
#include "fe/collision/ConvexCollider.h"
#include "fe/collision/ConcaveCollider.h"
#include "fe/collision/FineCollisionDetector.h"

namespace fe { namespace collision {

// Static variables definition
	const float FineCollisionDetector::sMinFDifference		= 0.00001f;
	const float FineCollisionDetector::sContactPrecision	= 0.0000001f;
	const float FineCollisionDetector::sContactSeparation	= 0.00001f;

// Public functions
	bool FineCollisionDetector::collide(
		const Collider* collider1, const Collider* collider2,
		Manifold& manifold
	) {
		if (!collider1 || !collider2) { return false; }

		if (auto convexCollider1 = dynamic_cast<const ConvexCollider*>(collider1)) {
			if (auto convexCollider2 = dynamic_cast<const ConvexCollider*>(collider2)) {
				return collideConvex(*convexCollider1, *convexCollider2, manifold);
			}
			else {
				auto concaveCollider2 = static_cast<const ConcaveCollider*>(collider2);
				return collideConvexConcave(*convexCollider1, *concaveCollider2, manifold, true);
			}
		}
		else {
			auto concaveCollider1 = static_cast<const ConcaveCollider*>(collider1);
			if (auto convexCollider2 = dynamic_cast<const ConvexCollider*>(collider2)) {
				return collideConvexConcave(*convexCollider2, *concaveCollider1, manifold, false);
			}
			else {
				auto concaveCollider2 = static_cast<const ConcaveCollider*>(collider2);
				return collideConcave(*concaveCollider1, *concaveCollider2, manifold);
			}
		}
	}

// Private functions
	bool FineCollisionDetector::collideConcave(
		const ConcaveCollider& collider1, const ConcaveCollider& collider2,
		Manifold& manifold
	) {
		bool anyCollides = false;

		// Remove the contacts that are no longer valid from the manifold
		removeInvalidContacts(manifold);

		// Get the overlapping convex parts of each concave collider
		auto overlappingParts1 = collider1.getOverlapingParts(collider2.getAABB());
		for (const std::unique_ptr<ConvexCollider>& part1 : overlappingParts1) {
			auto overlappingParts2 = collider2.getOverlapingParts(part1->getAABB());
			for (const std::unique_ptr<ConvexCollider>& part2 : overlappingParts2) {
				// GJK algorithm
				bool collides;
				std::vector<SupportPoint> simplex;
				std::tie(collides, simplex) = mGJKCollisionDetector.calculate(*part1, *part2);
				if (!collides) {
					continue;
				}

				// Create a polytope (tetrahedron) from the GJK simplex points
				Polytope polytope(*part1, *part2, simplex);

				// EPA Algorithm
				Contact newContact;
				if (!mEPACollisionDetector.calculate(*part1, *part2, polytope, newContact)) {
					continue;
				}

				// Check if the new Contact is far enough to the older contacts
				if (!isClose(newContact, manifold.mContacts)) {
					// Add the new contact to the manifold
					manifold.mContacts.push_back(newContact);
				}

				anyCollides = true;
			}
		}

		if (anyCollides) {
			// Limit the number of points in the manifold to 4
			limitManifoldContacts(manifold);
		}

		return anyCollides;
	}


	bool FineCollisionDetector::collideConvexConcave(
		const ConvexCollider& convexCollider, const ConcaveCollider& concaveCollider,
		Manifold& manifold, bool convexFirst
	) {
		bool anyCollides = false;

		// Remove the contacts that are no longer valid from the manifold
		removeInvalidContacts(manifold);

		// Get the overlapping convex parts of the concave collider with the
		// convex one
		auto overlappingParts = concaveCollider.getOverlapingParts(convexCollider.getAABB());
		for (const std::unique_ptr<ConvexCollider>& part : overlappingParts) {	
			// GJK algorithm
			bool collides;
			std::vector<SupportPoint> simplex;
			std::tie(collides, simplex) = (convexFirst)?
				mGJKCollisionDetector.calculate(convexCollider, *part) :
				mGJKCollisionDetector.calculate(*part, convexCollider);
			if (!collides) {
				continue;
			}

			// Create a polytope (tetrahedron) from the GJK simplex points
			Polytope polytope = (convexFirst)?
				Polytope(convexCollider, *part, simplex) :
				Polytope(*part, convexCollider, simplex);

			// EPA Algorithm
			Contact newContact;
			bool contactFilled = (convexFirst)?
				mEPACollisionDetector.calculate(convexCollider, *part, polytope, newContact) :
				mEPACollisionDetector.calculate(*part, convexCollider, polytope, newContact);
			if (!contactFilled) {
				continue;
			}

			// Check if the new Contact is far enough to the older contacts
			if (!isClose(newContact, manifold.mContacts)) {
				// Add the new contact to the manifold
				manifold.mContacts.push_back(newContact);
			}

			anyCollides = true;
		}

		if (anyCollides) {
			// Limit the number of points in the manifold to 4
			limitManifoldContacts(manifold);
		}

		return anyCollides;
	}


	bool FineCollisionDetector::collideConvex(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Manifold& manifold
	) {
		// GJK algorithm
		bool collides;
		std::vector<SupportPoint> simplex;
		std::tie(collides, simplex) = mGJKCollisionDetector.calculate(collider1, collider2);
		if (!collides) {
			return false;
		}

		// Create a polytope (tetrahedron) from the GJK simplex points
		Polytope polytope(collider1, collider2, simplex);

		// EPA Algorithm
		Contact newContact;
		if (!mEPACollisionDetector.calculate(collider1, collider2, polytope, newContact)) {
			return false;
		}

		// Remove the contacts that are no longer valid from the manifold
		removeInvalidContacts(manifold);

		// Check if the new Contact is far enough to the older contacts
		if (!isClose(newContact, manifold.mContacts)) {
			// Add the new contact to the manifold
			manifold.mContacts.push_back(newContact);

			// Limit the number of points in the manifold to 4
			limitManifoldContacts(manifold);
		}

		return true;
	}


	void FineCollisionDetector::removeInvalidContacts(Manifold& manifold) const
	{
		const glm::mat4 transforms1 = manifold.getFirstCollider()->getTransforms();
		const glm::mat4 transforms2 = manifold.getSecondCollider()->getTransforms();

		for (auto it = manifold.mContacts.begin(); it != manifold.mContacts.end();) {
			glm::vec3 changedWorldPos0 = transforms1 * glm::vec4(it->getLocalPosition(0), 1.0f);
			glm::vec3 changedWorldPos1 = transforms2 * glm::vec4(it->getLocalPosition(1), 1.0f);

			glm::vec3 v0 = it->getWorldPosition(0) - changedWorldPos0;
			glm::vec3 v1 = it->getWorldPosition(1) - changedWorldPos1;

			if ((glm::length(v0) >= sContactSeparation)
				|| (glm::length(v1) >= sContactSeparation)
			) {
				it = manifold.mContacts.erase(it);
			}
			else {
				++it;
			}
		}
	}


	bool FineCollisionDetector::isClose(
		const Contact& newContact,
		const std::vector<Contact>& contacts
	) const
	{
		for (const Contact& contact : contacts) {
			glm::vec3 v0 = newContact.getWorldPosition(0) - contact.getWorldPosition(0);
			glm::vec3 v1 = newContact.getWorldPosition(1) - contact.getWorldPosition(1);

			if ((glm::length(v0) < sContactSeparation) &&
				(glm::length(v1) < sContactSeparation)
			) {
				return true;
			}
		}

		return false;
	}


	void FineCollisionDetector::limitManifoldContacts(Manifold& manifold) const
	{
		if (manifold.mContacts.size() < 4) { return; }

		auto contact1 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[](const Contact& c1, const Contact& c2) {
				return c1.getPenetration() < c2.getPenetration();
			}
		);

		auto contact2 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[&](const Contact& c1, const Contact& c2) {
				float d1 = glm::length(c1.getWorldPosition(0) - contact1->getWorldPosition(0));
				float d2 = glm::length(c2.getWorldPosition(0) - contact1->getWorldPosition(0));
				return d1 < d2;
			}
		);

		auto contact3 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[&](const Contact& c1, const Contact& c2) {
				float d1 = distancePointEdge(
					c1.getWorldPosition(0),
					contact1->getWorldPosition(0), contact2->getWorldPosition(0)
				);
				float d2 = distancePointEdge(
					c2.getWorldPosition(0),
					contact1->getWorldPosition(0), contact2->getWorldPosition(0)
				);
				return d1 < d2;
			}
		);

		auto contact4 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[&](const Contact& c1, const Contact& c2) {
				float d1 = distancePointTriangle(
					c1.getWorldPosition(0),
					contact1->getWorldPosition(0), contact2->getWorldPosition(0), contact3->getWorldPosition(0)
				);
				float d2 = distancePointTriangle(
					c2.getWorldPosition(0),
					contact1->getWorldPosition(0), contact2->getWorldPosition(0), contact3->getWorldPosition(0)
				);
				return d1 < d2;
			}
		);

		manifold.mContacts = { *contact1, *contact2, *contact3, *contact4 };
	}


	float FineCollisionDetector::distancePointEdge(
		const glm::vec3& p,
		const glm::vec3& e1, const glm::vec3& e2
	) const
	{
		float ret = -1;

		glm::vec3 ve1p = p - e1, ve2p = p - e2, ve1e2 = glm::normalize(e2 - e1);
		if (float dot1 = glm::dot(ve1p, ve1e2) < 0) {
			ret = glm::length(ve1p);
		}
		else if (glm::dot(ve2p, ve1e2) > 0) {
			ret = glm::length(ve2p);
		}
		else {
			ret = glm::length(p - (e1 + dot1 * ve1e2));
		}

		return ret;
	}


	float FineCollisionDetector::distancePointTriangle(
		const glm::vec3& p,
		const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3
	) const
	{
		glm::vec3 vt1t2	= t1 - t2, vt2t3 = t2 - t3, vt3t1 = t3 - t1,
			tNormal = glm::normalize(-glm::cross(vt1t2, vt3t1));

		glm::vec3 vt1p = p - t1, vt1t2xtNormal = glm::normalize(glm::cross(vt1t2, tNormal));
		if (glm::dot(vt1p, vt1t2xtNormal) > 0) {
			return distancePointEdge(p, t1, t2);
		}

		glm::vec3 vt2p = p - t2, vt2t3xtNormal = glm::normalize(glm::cross(vt2t3, tNormal));
		if (glm::dot(vt2p, vt2t3xtNormal) > 0) {
			return distancePointEdge(p, t2, t3);
		}

		glm::vec3 vt3p = p - t3, vt3t1xtNormal = glm::normalize(glm::cross(vt3t1, tNormal));
		if (glm::dot(vt3p, vt3t1xtNormal) > 0) {
			return distancePointEdge(p, t3, t1);
		}

		return abs(glm::dot(vt1p, tNormal));
	}

}}
