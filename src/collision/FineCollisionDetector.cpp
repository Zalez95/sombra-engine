#include "FineCollisionDetector.h"
#include <limits>
#include <cassert>
#include <algorithm>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include "AABB.h"
#include "Contact.h"
#include "Manifold.h"
#include "Collider.h"
#include "ConvexCollider.h"
#include "ConcaveCollider.h"

namespace collision {

// Nested types definition
	struct FineCollisionDetector::SupportPoint
	{
		/** The SupportPoint's coordinates in the Configuration Space Object */
		glm::vec3 mV;

		/** The coordinates of the Contact point relative to each of the
		 * ConvexColliders in world space */
		glm::vec3 mWorldPos[2];

		/** The coordinates of the Contact point relative to each of the
		 * ConvexColliders in local space */
		glm::vec3 mLocalPos[2];

		SupportPoint() {};

		~SupportPoint() {};

		bool operator==(const SupportPoint& other) const
		{ return other.mV == mV; };
	};


	struct FineCollisionDetector::Edge
	{
		SupportPoint mA;
		SupportPoint mB;

		Edge(
			const SupportPoint& a,
			const SupportPoint& b
		) : mA(a), mB(b) {};

		~Edge() {};

		bool operator==(Edge e) { return (mA == e.mA) && (mB == e.mB); };
	};


	struct FineCollisionDetector::Triangle
	{
		SupportPoint mA;
		SupportPoint mB;
		SupportPoint mC;
		glm::vec3 mNormal;

		Triangle() {};

		Triangle(
			const SupportPoint& a,
			const SupportPoint& b,
			const SupportPoint& c
		) : mA(a), mB(b), mC(c)
		{ mNormal = glm::normalize(glm::cross(b.mV - a.mV, c.mV - a.mV)); };

		~Triangle() {};
	};

// Static variables definition
	const float FineCollisionDetector::CONTACT_TOLERANCE	= 0.0001f;
	const float FineCollisionDetector::CONTACT_SEPARATION	= 0.0002f;

// Public functions
	bool FineCollisionDetector::collide(
		const Collider* collider1, const Collider* collider2,
		Manifold& manifold
	) const
	{
		if (!collider1 || !collider2) { return false; }

		if (collider1->getType() == ColliderType::CONCAVE_COLLIDER) {
			auto concaveCollider1 = dynamic_cast<const ConcaveCollider*>(collider1);

			if (collider2->getType() == ColliderType::CONCAVE_COLLIDER) {
				auto concaveCollider2 = dynamic_cast<const ConcaveCollider*>(collider2);
				return collideConcave(*concaveCollider1, *concaveCollider2, manifold);
			}
			else {
				auto convexCollider2 = dynamic_cast<const ConvexCollider*>(collider2);
				// TODO: manifold order
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
	) const
	{
		bool collides = false;

		auto overlappingParts1 = collider1.getOverlapingParts(collider2.getAABB());
		for (const std::unique_ptr<ConvexCollider>& part : overlappingParts1) {
			collides = collides || collideConvexConcave(*part, collider2, manifold);
		}

		return collides;
	}


	bool FineCollisionDetector::collideConvexConcave(
		const ConvexCollider& convexCollider, const ConcaveCollider& concaveCollider,
		Manifold& manifold
	) const
	{
		bool collides = false;

		auto overlappingParts = concaveCollider.getOverlapingParts(convexCollider.getAABB());
		for (const std::unique_ptr<ConvexCollider>& part : overlappingParts) {
			collides = collides || collideConvex(convexCollider, *part, manifold);
		}

		return collides;
	}


	bool FineCollisionDetector::collideConvex(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Manifold& manifold
	) const
	{
		// GJK algorithm
		std::vector<SupportPoint> simplex;
		simplex.reserve(4);
		if (!calculateGJK(collider1, collider2, simplex)) {
			return false;
		}

		// Create a tetrahedron polytope from the simplex points
		auto polytope = createPolytope(collider1, collider2, simplex);

		// EPA Algorithm
		Contact newContact = calculateEPA(collider1, collider2, polytope);

		// Remove the contacts that are no longer valid from the manifold
		removeInvalidContacts(manifold);
		
		// Check if the new Contact is far enough to the older contacts
		if (!isClose(newContact, manifold)) {
			// Add the new contact to the manifold
			manifold.mContacts.push_back(newContact);

			// Limit the number of points in the manifold to 4
			limitManifoldContacts(manifold);
		}
		
		return true;
	}


	bool FineCollisionDetector::calculateGJK(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) const
	{
		// 1. Get an arbitrary point
		glm::vec3 direction = glm::sphericalRand(1.0f);
		simplex = { getSupportPoint(collider1, collider2, direction) };

		bool flag = !doSimplex(simplex, direction);
		while (flag) {
			// 2. Get a support point along the current direction
			SupportPoint supportPoint = getSupportPoint(collider1, collider2, direction);

			// 3. Check if the support point is further along the search direction
			if (glm::dot(supportPoint.mV, direction) > 0) {
				// 4.1 Add the point and update the simplex
				simplex.push_back(supportPoint);
				flag = !doSimplex(simplex, direction);
			}
			else {
				// 4.2 There is no collision, exit without finishing the simplex
				return false;
			}
		}

		return true;
	}


	std::vector<FineCollisionDetector::Triangle> FineCollisionDetector::createPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) const
	{
		assert(simplex.size() > 0 && "The simplex must have at least one support point.");

		if (simplex.size() == 1) {
			// Search a support point in each axis direction
			for (unsigned int i = 0; i < 3; ++i) {
				for (float v : {-1.0f, 1.0f}) {
					glm::vec3 searchDir;
					searchDir[i] = v;
					SupportPoint sp = getSupportPoint(collider1, collider2, searchDir);

					if (glm::length(sp.mV - simplex[0].mV) >= CONTACT_TOLERANCE) {
						simplex.push_back(sp);
						break;
					}
				}
			}
		}
		if (simplex.size() == 2) {
			glm::vec3 v01 = simplex[1].mV - simplex[0].mV;

			// Calculate a rotation matrix of 60 degrees around the line vector
			glm::mat3 rotate60 = glm::mat3(glm::rotate(glm::mat4(), glm::pi<float>() / 6.0f, v01));

			// Find the least significant axis
			unsigned int closestAxis = 0;
			for (unsigned int i = 1; i < 3; ++i) {
				if (v01[i] < v01[closestAxis]) {
					closestAxis = i;
				}
			}

			glm::vec3 axis(0.0f);
			axis[closestAxis] = 1.0f;

			// Calculate a normal vector to the line with the axis
			glm::vec3 vNormal = glm::cross(v01, axis);

			// Search a support point in the 6 directions around the line
			glm::vec3 searchDir = vNormal;
			for (size_t i = 0; i < 6; ++i) {
				SupportPoint sp = getSupportPoint(collider1, collider2, searchDir);

				if (glm::length(sp.mV) > CONTACT_TOLERANCE) {
					simplex.push_back(sp);
					break;
				}

				searchDir = rotate60 * searchDir;
			}
		}
		if (simplex.size() == 3) {
			// Search a support point along the simplex's triangle normal
			glm::vec3 v01 = simplex[1].mV - simplex[0].mV;
			glm::vec3 v02 = simplex[2].mV - simplex[0].mV;
			glm::vec3 searchDir = glm::cross(v01, v02);

			SupportPoint sp = getSupportPoint(collider1, collider2, searchDir);

			if (glm::length(sp.mV) <= CONTACT_TOLERANCE) {
				// Try the opposite direction
				searchDir = -searchDir;
				sp = getSupportPoint(collider1, collider2, searchDir);
			}

			simplex.push_back(sp);
		}

		// Create the polytope from the simplex's points
		SupportPoint &d = simplex[0], &c = simplex[1], &b = simplex[2], &a = simplex[3];
		return { Triangle(a,b,c), Triangle(a,d,b), Triangle(a,c,d), Triangle(b,d,c) };
	}


	Contact FineCollisionDetector::calculateEPA(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<Triangle>& polytope
	) const
	{
		std::vector<Triangle>::iterator closestF;
		float closestFDist = std::numeric_limits<float>::max();
		while (true) {
			// 1. Calculate the closest face to the origin of the polytope
			auto closestF2			= polytope.begin();
			float closestFDist2		= std::numeric_limits<float>::max();
			for (auto it = polytope.begin(); it != polytope.end(); ++it) {
				float fDist = abs(glm::dot(it->mNormal, it->mA.mV));
				if (fDist <= closestFDist2) {
					closestF2		= it;
					closestFDist2	= fDist;
				}
			}

			// 2. If the difference of distance to the origin between the
			// current closest face and last one is smaller than TOLERANCE
			// we have found the closest triangle
			if (closestFDist - closestFDist2 <= CONTACT_TOLERANCE) {
				closestF			= closestF2;
				closestFDist		= closestFDist2;
				break;
			}

			// 3. Get a new support point along the face normal
			SupportPoint supportPoint = getSupportPoint(collider1, collider2, closestF2->mNormal);

			// 4. Remove the current closest face from the polytope
			polytope.erase(closestF2);

			// 5. Delete the faces that can be seen from the new point and get
			// the edges of the created hole
			std::vector<Edge> holeEdges;
			for (auto it = polytope.begin(); it != polytope.end();) {
				if (glm::dot(it->mNormal, supportPoint.mV) > 0) {
					Edge e1(it->mA, it->mB);
					auto itE1 = std::find(holeEdges.begin(), holeEdges.end(), e1);
					if (itE1 == holeEdges.end()) holeEdges.push_back(e1); else holeEdges.erase(itE1);

					Edge e2(it->mB, it->mC);
					auto itE2 = std::find(holeEdges.begin(), holeEdges.end(), e2);
					if (itE2 == holeEdges.end()) holeEdges.push_back(e2); else holeEdges.erase(itE2);

					Edge e3(it->mC, it->mA);
					auto itE3 = std::find(holeEdges.begin(), holeEdges.end(), e3);
					if (itE3 == holeEdges.end()) holeEdges.push_back(e3); else holeEdges.erase(itE3);

					it = polytope.erase(it);
				}
				else {
					++it;
				}
			}

			// 6. Add new faces connecting the edges of the hole to the support point
			for (Edge e : holeEdges) { polytope.emplace_back(supportPoint, e.mA, e.mB); }

			closestFDist = closestFDist2;
		}

		// 7. Project The origin into the closest triangle and get its
		// barycentric coordinates
		glm::vec3 baryPosition;
		glm::intersectRayTriangle(
			glm::vec3(0), closestF->mNormal,
			closestF->mA.mV, closestF->mB.mV, closestF->mC.mV,
			baryPosition
		);
		baryPosition.z = 1.0f - baryPosition.x - baryPosition.y;

		// 8. Calculate the coordinates of the contact from the barycenter
		// coordinates of the point
		Contact ret(closestFDist, -closestF->mNormal);
		for (unsigned int i = 0; i < 2; ++i) {
			for (unsigned int j = 0; j < 1; ++j) {
				ret.mWorldPos[i][j] = baryPosition.x * closestF->mA.mWorldPos[i][j]
									+ baryPosition.y * closestF->mB.mWorldPos[i][j]
									+ baryPosition.z * closestF->mC.mWorldPos[i][j];
				ret.mLocalPos[i][j] = baryPosition.x * closestF->mA.mLocalPos[i][j]
									+ baryPosition.y * closestF->mB.mLocalPos[i][j]
									+ baryPosition.z * closestF->mC.mLocalPos[i][j];
			}
		}

		return ret;
	}


	void FineCollisionDetector::removeInvalidContacts(Manifold& manifold) const
	{
		glm::mat4 transforms1 = manifold.getFirstCollider()->getTransforms();
		glm::mat4 transforms2 = manifold.getSecondCollider()->getTransforms();
		
		for (auto it = manifold.mContacts.begin(); it != manifold.mContacts.end();) {
			glm::vec3 changedWorldPos0(transforms1 * glm::vec4(it->getLocalPosition(0), 1.0f));
			glm::vec3 changedWorldPos1(transforms2 * glm::vec4(it->getLocalPosition(1), 1.0f));

			glm::vec3 v0 = it->getWorldPosition(0) - changedWorldPos0;
			glm::vec3 v1 = it->getWorldPosition(1) - changedWorldPos1;

			if ((glm::length(v0) >= CONTACT_SEPARATION)
				|| (glm::length(v1) >= CONTACT_SEPARATION)
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
		const Manifold& manifold
	) const
	{
		for (const Contact& contact : manifold.getContacts()) {
			glm::vec3 v0 = newContact.getWorldPosition(0) - contact.getWorldPosition(0);
			glm::vec3 v1 = newContact.getWorldPosition(1) - contact.getWorldPosition(1);

			if ((glm::length(v0) < CONTACT_SEPARATION) &&
				(glm::length(v1) < CONTACT_SEPARATION)
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
			[contact1](const Contact& c1, const Contact& c2) {
				float d1 = glm::length(c1.getWorldPosition(0) - contact1->getWorldPosition(0));
				float d2 = glm::length(c2.getWorldPosition(0) - contact1->getWorldPosition(0));
				return d1 < d2;
			}
		);

		auto contact3 = std::max_element(
			manifold.mContacts.begin(), manifold.mContacts.end(),
			[contact1, contact2](const Contact& c1, const Contact& c2) {
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
			[contact1, contact2, contact3](const Contact& c1, const Contact& c2) {
				glm::vec3 v12	= contact1->getWorldPosition(0) - contact2->getWorldPosition(0);
				//TODO:
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


	FineCollisionDetector::SupportPoint FineCollisionDetector::getSupportPoint(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const glm::vec3& direction
	) const
	{
		SupportPoint ret;
		
		collider1.getFurthestPointInDirection(direction, ret.mWorldPos[0], ret.mLocalPos[0]);
		collider2.getFurthestPointInDirection(-direction, ret.mWorldPos[1], ret.mLocalPos[1]);
		ret.mV = ret.mWorldPos[0] - ret.mWorldPos[1];

		return ret;
	}


	bool FineCollisionDetector::doSimplex(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		assert(!simplex.empty() && "The simplex has to have at least one initial point");

		bool ret;
		switch (simplex.size() - 1) {
			case 0:
				ret = doSimplex0D(simplex, searchDir);
				break;
			case 1:
				ret = doSimplex1D(simplex, searchDir);
				break;
			case 2:
				ret = doSimplex2D(simplex, searchDir);
				break;
			case 3:
				ret = doSimplex3D(simplex, searchDir);
				break;
			default:
				ret = false;
		}

		return ret;
	}


	bool FineCollisionDetector::doSimplex0D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint a = simplex[0];
		simplex = { a };
		searchDir = -a.mV;

		// Check if the support point is the origin
		ret = (a.mV == glm::vec3(0));

		return ret;
	}


	bool FineCollisionDetector::doSimplex1D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint b = simplex[0], a = simplex[1];
		glm::vec3 ab = b.mV - a.mV, ao = -a.mV;

		float dot = glm::dot(ab, ao);
		if (dot >= 0) {
			// The origin is between b and a
			simplex = { b, a };
			searchDir = glm::cross(glm::cross(ab, ao), ab);

			// Check if the origin is on the line
			ret = (dot == 0.0f);
		}
		else {
			// Discard b and do the same than with 0 dimensions
			simplex = { a };
			ret = doSimplex0D(simplex, searchDir);
		}

		return ret;
	}


	bool FineCollisionDetector::doSimplex2D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint c = simplex[0], b = simplex[1], a = simplex[2];
		glm::vec3	ab = b.mV - a.mV, ac = c.mV - a.mV, ao = -a.mV,
					abc = glm::cross(ab, ac);

		if (glm::dot(glm::cross(ab, abc), ao) > 0) {
			// Origin outside the triangle from the ab edge
			// Discard c point and test the edge in 1 dimension
			simplex = { b, a };
			ret = doSimplex1D(simplex, searchDir);
		}
		else {
			if (glm::dot(glm::cross(abc, ac), ao) > 0) {
				// Origin outside the triangle from the ac edge
				// Discard b point and test the edge in 1 dimension
				simplex = { c, a };
				ret = doSimplex1D(simplex, searchDir);
			}
			else {
				// Inside the triangle in 2D
				// Check if the origin is above or below the triangle
				float dot = glm::dot(abc, ao);
				if (dot >= 0) {
					simplex = { c, b, a };
					searchDir = abc;

					// Check if the origin is on the triangle
					ret = (dot == 0.0f);
				}
				else {
					simplex = { b, c, a };
					searchDir = -abc;
				}
			}
		}

		return ret;
	}


	bool FineCollisionDetector::doSimplex3D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint d = simplex[0], c = simplex[1], b = simplex[2], a = simplex[3];
		glm::vec3	ab = b.mV - a.mV, ac = c.mV - a.mV, ad = d.mV - a.mV, ao = -a.mV,
					abc = glm::cross(ab, ac), acd = glm::cross(ac, ad), adb = glm::cross(ad, ab);

		if (glm::dot(abc, ao) > 0) {
			// Origin outside the tetrahedron from the abc face
			// Discard d and check the triangle in 2 dimensions
			simplex = { c, b, a };
			ret = doSimplex2D(simplex, searchDir);
		}
		else {
			if (glm::dot(acd, ao) > 0) {
				// Origin outside the tetrahedron from the acd face
				// Discard b and check the triangle in 2 dimensions
				simplex = { d, c, a };
				ret = doSimplex2D(simplex, searchDir);
			}
			else {
				if (glm::dot(adb, ao) > 0) {
					// Origin outside the tetrahedron from the adb face
					// Discard c and check the triangle in 2 dimensions
					simplex = { b, d, a };
					ret = doSimplex2D(simplex, searchDir);
				}
				else {
					// Inside the tetrahedron
					ret = true;
				}
			}
		}

		return ret;
	}

}
