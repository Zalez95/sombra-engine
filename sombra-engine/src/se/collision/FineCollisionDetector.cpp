#include <algorithm>
#include "se/utils/MathUtils.h"
#include "se/collision/Contact.h"
#include "se/collision/Manifold.h"
#include "se/collision/Collider.h"
#include "se/collision/ConvexCollider.h"
#include "se/collision/ConcaveCollider.h"
#include "se/collision/FineCollisionDetector.h"

namespace se::collision {

	FineCollisionDetector::FineCollisionDetector(
		float minFDifference, std::size_t maxIterations,
		float contactPrecision, float contactSeparation
	) : mGJKCollisionDetector(contactPrecision, maxIterations),
		mEPACollisionDetector(minFDifference, maxIterations, contactPrecision),
		mContactSeparation2(contactSeparation * contactSeparation) {}


	bool FineCollisionDetector::collide(Manifold& manifold) const
	{
		const Collider* collider1 = manifold.colliders[0];
		const Collider* collider2 = manifold.colliders[1];
		if (!collider1 || !collider2) {
			return false;
		}

		// Skip non updated Colliders
		if (!collider1->updated() && !collider2->updated()) {
			return manifold.state[Manifold::State::Intersecting];
		}

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
		auto [collides, simplex] = mGJKCollisionDetector.calculateIntersection(collider1, collider2);
		if (!collides) {
			manifold.contacts.clear();
			manifold.state.reset(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
			return false;
		}

		// EPA Algorithm
		auto [success, contact] = mEPACollisionDetector.calculate(collider1, collider2, simplex);
		if (!success) {
			manifold.contacts.clear();
			manifold.state.reset(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
			return false;
		}

		// Remove the contacts that are no longer valid from the manifold
		removeInvalidContacts(manifold);

		// Add the new contact to the manifold
		addContact(contact, manifold);

		if (!manifold.state[Manifold::State::Intersecting]) {
			manifold.state.set(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
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
		concaveCollider.processOverlapingParts(convexCollider.getAABB(), [&](const ConvexCollider& part) {
			// GJK algorithm
			auto [collides, simplex] = (convexFirst)?
				mGJKCollisionDetector.calculateIntersection(convexCollider, part) :
				mGJKCollisionDetector.calculateIntersection(part, convexCollider);
			if (!collides) {
				return;
			}

			// EPA Algorithm
			auto [success, contact] = (convexFirst)?
				mEPACollisionDetector.calculate(convexCollider, part, simplex) :
				mEPACollisionDetector.calculate(part, convexCollider, simplex);
			if (!success) {
				return;
			}

			nNewContacts++;
			if (nNewContacts == 1) {
				// Remove the old contacts that are no longer valid from the
				// manifold
				removeInvalidContacts(manifold);
			}

			// Add the new contact to the manifold
			addContact(contact, manifold);
		});

		if (nNewContacts == 0) {
			manifold.contacts.clear();
			manifold.state.reset(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
			return false;
		}

		if (!manifold.state[Manifold::State::Intersecting]) {
			manifold.state.set(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
		}

		return true;
	}


	bool FineCollisionDetector::collideConcave(
		const ConcaveCollider& collider1, const ConcaveCollider& collider2,
		Manifold& manifold
	) const
	{
		int nNewContacts = 0;

		// Get the overlapping convex parts of each concave collider
		collider1.processOverlapingParts(collider2.getAABB(), [&](const ConvexCollider& part1) {
			collider2.processOverlapingParts(part1.getAABB(), [&](const ConvexCollider& part2) {
				// GJK algorithm
				auto [collides, simplex] = mGJKCollisionDetector.calculateIntersection(part1, part2);
				if (!collides) {
					return;
				}

				// EPA Algorithm
				auto [success, contact] = mEPACollisionDetector.calculate(part1, part2, simplex);
				if (!success) {
					return;
				}

				nNewContacts++;
				if (nNewContacts == 1) {
					// Remove the old contacts that are no longer valid from the
					// manifold
					removeInvalidContacts(manifold);
				}

				// Add the new contact to the manifold
				addContact(contact, manifold);
			});
		});

		if (nNewContacts == 0) {
			manifold.contacts.clear();
			manifold.state.reset(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
			return false;
		}

		if (!manifold.state[Manifold::State::Intersecting]) {
			manifold.state.set(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
		}

		return true;
	}


	void FineCollisionDetector::addContact(Contact& contact, Manifold& manifold) const
	{
		// Check if the Contact is far enough from the Manifold contacts
		if (!isClose(contact, manifold.contacts.data(), manifold.contacts.size())) {
			if (manifold.contacts.size() < Manifold::kMaxContacts) {
				// Add the new contact to the manifold
				manifold.contacts.push_back(contact);
			}
			else {
				// Limit the number of Contacts to 4
				auto limitedContacts = limitManifoldContacts({
					&manifold.contacts[0], &manifold.contacts[1],
					&manifold.contacts[2], &manifold.contacts[3],
					&contact
				});

				for (std::size_t i = 0; i < manifold.contacts.size(); ++i) {
					manifold.contacts[i] = *limitedContacts[i];
				}
			}

			manifold.state.set(Manifold::State::Updated);
		}
	}


	void FineCollisionDetector::removeInvalidContacts(Manifold& manifold) const
	{
		const glm::mat4 transforms1 = manifold.colliders[0]->getTransforms();
		const glm::mat4 transforms2 = manifold.colliders[1]->getTransforms();

		for (std::size_t i = 0; i < manifold.contacts.size();) {
			glm::vec3 changedWorldPos0 = transforms1 * glm::vec4(manifold.contacts[i].localPosition[0], 1.0f);
			glm::vec3 changedWorldPos1 = transforms2 * glm::vec4(manifold.contacts[i].localPosition[1], 1.0f);

			glm::vec3 v0 = manifold.contacts[i].worldPosition[0] - changedWorldPos0;
			glm::vec3 v1 = manifold.contacts[i].worldPosition[1] - changedWorldPos1;

			if ((glm::dot(v0, v0) >= mContactSeparation2) || (glm::dot(v1, v1) >= mContactSeparation2)) {
				if (i + 1 < manifold.contacts.size()) {
					std::swap(manifold.contacts[i], manifold.contacts.back());
				}
				manifold.contacts.pop_back();
				manifold.state.set(Manifold::State::Updated);
			}
			else {
				++i;
			}
		}
	}


	bool FineCollisionDetector::isClose(
		const Contact& newContact,
		const Contact* contacts, std::size_t numContacts
	) const
	{
		return (numContacts > 0)
			&& std::any_of(
				&contacts[0], &contacts[numContacts-1],
				[&](const Contact& contact) {
					glm::vec3 v0 = newContact.worldPosition[0] - contact.worldPosition[0];
					glm::vec3 v1 = newContact.worldPosition[1] - contact.worldPosition[1];
					return ((glm::dot(v0, v0) < mContactSeparation2) && (glm::dot(v1, v1) < mContactSeparation2));
				}
			);
	}


	std::array<Contact*, 4> FineCollisionDetector::limitManifoldContacts(
		const std::array<Contact*, 5>& contacts
	) {
		Contact* contact1 = *std::max_element(
			contacts.begin(), contacts.end(),
			[](const Contact* c1, const Contact* c2) {
				return c1->penetration < c2->penetration;
			}
		);

		Contact* contact2 = *std::max_element(
			contacts.begin(), contacts.end(),
			[&](const Contact* c1, const Contact* c2) {
				glm::vec3 contact1c1 = c1->worldPosition[0] - contact1->worldPosition[0];
				glm::vec3 contact1c2 = c2->worldPosition[0] - contact1->worldPosition[0];
				return glm::dot(contact1c1, contact1c1) < glm::dot(contact1c2, contact1c2);
			}
		);

		Contact* contact3 = *std::max_element(
			contacts.begin(), contacts.end(),
			[&](const Contact* c1, const Contact* c2) {
				glm::vec3 vDist1 = c1->worldPosition[0] - utils::getClosestPointInEdge(
					c1->worldPosition[0],
					contact1->worldPosition[0], contact2->worldPosition[0]
				);
				glm::vec3 vDist2 = c2->worldPosition[0] - utils::getClosestPointInEdge(
					c2->worldPosition[0],
					contact1->worldPosition[0], contact2->worldPosition[0]
				);
				return glm::dot(vDist1, vDist1) < glm::dot(vDist2, vDist2);
			}
		);

		Contact* contact4 = *std::max_element(
			contacts.begin(), contacts.end(),
			[&](const Contact* c1, const Contact* c2) {
				glm::vec3 vDist1 = c1->worldPosition[0] - utils::getClosestPointInPlane(
					c1->worldPosition[0],
					{ contact1->worldPosition[0], contact2->worldPosition[0], contact3->worldPosition[0] }
				);
				glm::vec3 vDist2 = c2->worldPosition[0] - utils::getClosestPointInPlane(
					c2->worldPosition[0],
					{ contact1->worldPosition[0], contact2->worldPosition[0], contact3->worldPosition[0] }
				);
				return glm::dot(vDist1, vDist1) < glm::dot(vDist2, vDist2);
			}
		);

		return { contact1, contact2, contact3, contact4 };
	}

}
