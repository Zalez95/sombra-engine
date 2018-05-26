#include <tuple>
#include <algorithm>
#include "fe/collision/Contact.h"
#include "fe/collision/Manifold.h"
#include "fe/collision/Collider.h"
#include "fe/collision/ConvexCollider.h"
#include "fe/collision/ConcaveCollider.h"
#include "fe/collision/FineCollisionDetector.h"
#include "Edge.h"
#include "Triangle.h"

namespace fe { namespace collision {

	bool FineCollisionDetector::collide(Manifold& manifold) const
	{
		const Collider* collider1 = manifold.mColliders[0];
		const Collider* collider2 = manifold.mColliders[1];
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
	bool FineCollisionDetector::collideConvex(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Manifold& manifold
	) const
	{
		// GJK algorithm
		bool collides;
		std::vector<SupportPoint> simplex;
		std::tie(collides, simplex) = mGJKCollisionDetector.calculate(collider1, collider2);
		if (!collides) {
			return false;
		}

		// EPA Algorithm
		Contact newContact;
		if (!mEPACollisionDetector.calculate(collider1, collider2, simplex, newContact)) {
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


	bool FineCollisionDetector::collideConvexConcave(
		const ConvexCollider& convexCollider, const ConcaveCollider& concaveCollider,
		Manifold& manifold, bool convexFirst
	) const
	{
		int nNewContacts = 0;

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

			// EPA Algorithm
			Contact newContact;
			bool contactFilled = (convexFirst)?
				mEPACollisionDetector.calculate(convexCollider, *part, simplex, newContact) :
				mEPACollisionDetector.calculate(*part, convexCollider, simplex, newContact);
			if (!contactFilled) {
				continue;
			}

			if (nNewContacts == 0) {
				// Remove the old contacts that are no longer valid from the
				// manifold
				removeInvalidContacts(manifold);
			}

			// Check if the new Contact is far enough to the older contacts
			if (!isClose(newContact, manifold.mContacts)) {
				// Add the new contact to the manifold
				manifold.mContacts.push_back(newContact);
				nNewContacts++;
			}
		}

		if (nNewContacts > 0) {
			// Limit the number of points in the manifold to 4
			limitManifoldContacts(manifold);
			return true;
		}

		return false;
	}


	bool FineCollisionDetector::collideConcave(
		const ConcaveCollider& collider1, const ConcaveCollider& collider2,
		Manifold& manifold
	) const
	{
		int nNewContacts = 0;

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

				// EPA Algorithm
				Contact newContact;
				if (!mEPACollisionDetector.calculate(*part1, *part2, simplex, newContact)) {
					continue;
				}

				if (nNewContacts == 0) {
					// Remove the old contacts that are no longer valid from the
					// manifold
					removeInvalidContacts(manifold);
				}

				// Check if the new Contact is far enough to the older contacts
				if (!isClose(newContact, manifold.mContacts)) {
					// Add the new contact to the manifold
					manifold.mContacts.push_back(newContact);
					nNewContacts++;
				}
			}
		}

		if (nNewContacts > 0) {
			// Limit the number of points in the manifold to 4
			limitManifoldContacts(manifold);

			return true;
		}

		return false;
	}


	void FineCollisionDetector::removeInvalidContacts(Manifold& manifold) const
	{
		const glm::mat4 transforms1 = manifold.mColliders[0]->getTransforms();
		const glm::mat4 transforms2 = manifold.mColliders[1]->getTransforms();

		for (auto it = manifold.mContacts.begin(); it != manifold.mContacts.end();) {
			glm::vec3 changedWorldPos0 = transforms1 * glm::vec4(it->getLocalPosition(0), 1.0f);
			glm::vec3 changedWorldPos1 = transforms2 * glm::vec4(it->getLocalPosition(1), 1.0f);

			glm::vec3 v0 = it->getWorldPosition(0) - changedWorldPos0;
			glm::vec3 v1 = it->getWorldPosition(1) - changedWorldPos1;

			if ((glm::length(v0) >= mContactSeparation)
				|| (glm::length(v1) >= mContactSeparation)
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

			if ((glm::length(v0) < mContactSeparation) &&
				(glm::length(v1) < mContactSeparation)
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
				glm::vec3 p1 = getClosestPointInPlane(
					c1.getWorldPosition(0),
					{ contact1->getWorldPosition(0), contact2->getWorldPosition(0), contact3->getWorldPosition(0) }
				);
				glm::vec3 p2 = getClosestPointInPlane(
					c2.getWorldPosition(0),
					{ contact1->getWorldPosition(0), contact2->getWorldPosition(0), contact3->getWorldPosition(0) }
				);
				return glm::length(p1) < glm::length(p2);
			}
		);

		manifold.mContacts = { *contact1, *contact2, *contact3, *contact4 };
	}

}}
