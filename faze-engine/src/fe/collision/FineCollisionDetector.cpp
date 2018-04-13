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

		if (collider1->getType() == ColliderType::CONCAVE_COLLIDER) {
			auto concaveCollider1 = dynamic_cast<const ConcaveCollider*>(collider1);

			if (collider2->getType() == ColliderType::CONCAVE_COLLIDER) {
				auto concaveCollider2 = dynamic_cast<const ConcaveCollider*>(collider2);
				return collideConcave(*concaveCollider1, *concaveCollider2, manifold);
			}
			else {
				auto convexCollider2 = dynamic_cast<const ConvexCollider*>(collider2);
				// FIXME: manifold order
				return collideConvexConcave(*convexCollider2, *concaveCollider1, manifold);
			}
		}
		else {
			auto convexCollider1 = dynamic_cast<const ConvexCollider*>(collider1);

			if (collider2->getType() == ColliderType::CONCAVE_COLLIDER) {
				auto concaveCollider2 = dynamic_cast<const ConcaveCollider*>(collider2);
				return collideConvexConcave(*convexCollider1, *concaveCollider2, manifold);
			}
			else {
				auto convexCollider2 = dynamic_cast<const ConvexCollider*>(collider2);
				return collideConvex(*convexCollider1, *convexCollider2, manifold);
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
				Contact newContact = mEPACollisionDetector.calculate(*part1, *part2, polytope);

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
		Manifold& manifold
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
			std::tie(collides, simplex) = mGJKCollisionDetector.calculate(convexCollider, *part);
			if (!collides) {
				continue;
			}

			// Create a polytope (tetrahedron) from the GJK simplex points
			Polytope polytope(convexCollider, *part, simplex);

			// EPA Algorithm
			Contact newContact = mEPACollisionDetector.calculate(convexCollider, *part, polytope);

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
		Contact newContact = mEPACollisionDetector.calculate(collider1, collider2, polytope);

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
			[&contact1](const Contact& c1, const Contact& c2) {
				float d1 = glm::length(c1.getWorldPosition(0) - contact1->getWorldPosition(0));
				float d2 = glm::length(c2.getWorldPosition(0) - contact1->getWorldPosition(0));
				return d1 < d2;
			}
		);

		auto contact3 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[&contact1, &contact2](const Contact& c1, const Contact& c2) {
				glm::vec3 v12	= contact1->getWorldPosition(0) - contact2->getWorldPosition(0);
				glm::vec3 v1c1	= c1.getWorldPosition(0) - contact1->getWorldPosition(0);
				glm::vec3 v1c2	= c2.getWorldPosition(0) - contact1->getWorldPosition(0);

				float d1 = glm::length(glm::cross(v12, v1c1));
				float d2 = glm::length(glm::cross(v12, v1c2));
				return d1 < d2;
			}
		);

		auto contact4 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[&contact1, &contact2, &contact3](const Contact& c1, const Contact& c2) {
				// FIXME: this is wrong
				glm::vec3 v12	= contact1->getWorldPosition(0) - contact2->getWorldPosition(0);

				//glm::vec3 v13	= contact1->getWorldPosition(0) - contact3->getWorldPosition(0);
				//glm::vec3 vN	= glm::cross(v12, v13);
				glm::vec3 v1c1	= c1.getWorldPosition(0) - contact1->getWorldPosition(0);
				glm::vec3 v1c2	= c2.getWorldPosition(0) - contact1->getWorldPosition(0);

				float d1 = glm::length(glm::cross(v12, v1c1));
				float d2 = glm::length(glm::cross(v12, v1c2));
				return d1 < d2;
			}
		);

		manifold.mContacts = { *contact1, *contact2, *contact3, *contact4 };
	}

}}
