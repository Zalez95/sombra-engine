#include <algorithm>
#include "se/utils/MathUtils.h"
#include "se/physics/collision/Contact.h"
#include "se/physics/collision/Manifold.h"
#include "se/physics/collision/Collider.h"
#include "se/physics/collision/ConvexCollider.h"
#include "se/physics/collision/ConcaveCollider.h"
#include "se/physics/collision/FineCollisionDetector.h"
#include "GJKCollisionDetector.h"
#include "EPACollisionDetector.h"
#include "GJKRayCaster.h"

namespace se::physics {

	FineCollisionDetector::FineCollisionDetector(
		float coarseEpsilon,
		float minFDifference, std::size_t maxIterations,
		float contactPrecision, float contactSeparation,
		float raycastPrecision
	) : mGJKCollisionDetector( std::make_unique<GJKCollisionDetector>(contactPrecision, maxIterations) ),
		mEPACollisionDetector( std::make_unique<EPACollisionDetector>(minFDifference, maxIterations, contactPrecision) ),
		mGJKRayCaster( std::make_unique<GJKRayCaster>(raycastPrecision, maxIterations) ),
		mCoarseEpsilon(coarseEpsilon), mContactSeparation2(contactSeparation * contactSeparation) {}


	FineCollisionDetector::~FineCollisionDetector() {}


	bool FineCollisionDetector::collide(Manifold& manifold)
	{
		const Collider* collider1 = manifold.colliders[0];
		const Collider* collider2 = manifold.colliders[1];
		if (!collider1 || !collider2) {
			return false;
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


	std::pair<bool, RayCast> FineCollisionDetector::intersects(
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		const Collider& collider
	) {
		bool intersects = false;
		RayCast rayCast;
		rayCast.distance = std::numeric_limits<float>::max();

		if (auto convexCollider = dynamic_cast<const ConvexCollider*>(&collider)) {
			std::tie(intersects, rayCast) = mGJKRayCaster->calculateRayCast(rayOrigin, rayDirection, *convexCollider);
		}
		else if (auto concaveCollider = dynamic_cast<const ConcaveCollider*>(&collider)) {
			concaveCollider->processIntersectingParts(
				rayOrigin, rayDirection, mCoarseEpsilon,
				[&](const ConvexCollider& convexCollider2) {
					auto [intersects2, rayCast2] = mGJKRayCaster->calculateRayCast(rayOrigin, rayDirection, convexCollider2);
					if (intersects2 && (!intersects || (rayCast2.distance < rayCast.distance))) {
						intersects = intersects2;
						rayCast = rayCast2;
					}
				}
			);
		}

		return { intersects, rayCast };
	}

// Private functions
	bool FineCollisionDetector::collideConvex(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Manifold& manifold
	) {
		// GJK algorithm
		auto [collides, simplex] = mGJKCollisionDetector->calculateIntersection(collider1, collider2);
		if (!collides) {
			manifold.contacts.clear();
			manifold.state.reset(Manifold::State::Intersecting);
			manifold.state.set(Manifold::State::Updated);
			return false;
		}

		// EPA Algorithm
		auto [success, contact] = mEPACollisionDetector->calculate(collider1, collider2, simplex);
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
	) {
		int nNewContacts = 0;

		// Get the overlapping convex parts of the concave collider with the
		// convex one
		concaveCollider.processOverlapingParts(convexCollider.getAABB(), mCoarseEpsilon, [&](const ConvexCollider& part) {
			// GJK algorithm
			auto [collides, simplex] = (convexFirst)?
				mGJKCollisionDetector->calculateIntersection(convexCollider, part) :
				mGJKCollisionDetector->calculateIntersection(part, convexCollider);
			if (!collides) {
				return;
			}

			// EPA Algorithm
			auto [success, contact] = (convexFirst)?
				mEPACollisionDetector->calculate(convexCollider, part, simplex) :
				mEPACollisionDetector->calculate(part, convexCollider, simplex);
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
	) {
		int nNewContacts = 0;

		// Get the overlapping convex parts of each concave collider
		collider1.processOverlapingParts(collider2.getAABB(), mCoarseEpsilon, [&](const ConvexCollider& part1) {
			collider2.processOverlapingParts(part1.getAABB(), mCoarseEpsilon, [&](const ConvexCollider& part2) {
				// GJK algorithm
				auto [collides, simplex] = mGJKCollisionDetector->calculateIntersection(part1, part2);
				if (!collides) {
					return;
				}

				// EPA Algorithm
				auto [success, contact] = mEPACollisionDetector->calculate(part1, part2, simplex);
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
