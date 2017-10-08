#include "FineCollisionDetector.h"
#include <limits>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <glm/gtc/random.hpp>
#include <glm/gtx/intersect.hpp>
#include "Contact.h"
#include "Collider.h"
#include "BoundingSphere.h"
#include "ConvexPolyhedron.h"

namespace collision {

// Nested types definition
	struct FineCollisionDetector::SupportPoint
	{
		// The SupportPoint's coordinates in the Configuration Space Object
		glm::vec3 mV;

		// The SupportPoint's coordinates relative to the first Collider
		glm::vec3 mP1;

		// The SupportPoint's coordinates relative to the second Collider
		glm::vec3 mP2;

		SupportPoint() {};

		SupportPoint(
			const glm::vec3& v,
			const glm::vec3& p1, const glm::vec3& p2
		) : mV(v), mP1(p1), mP2(p2) {};

		~SupportPoint() {};

		bool operator==(const SupportPoint& other) const
		{ return other.mV == mV; };
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

// Static variables definition
	const float FineCollisionDetector::TOLERANCE = 0.001f;

// Public functions
	std::vector<Contact> FineCollisionDetector::collide(
		const Collider& collider1,
		const Collider& collider2
	) const
	{
		std::vector<Contact> ret;

		// GJK algorithm
		std::vector<SupportPoint> simplex;
		if (!calculateGJK(collider1, collider2, simplex)) {
			return ret;
		}

		// EPA Algorithm
		// Initialize the polytope with the simplex points
		SupportPoint &d = simplex[0], &c = simplex[1], &b = simplex[2], &a = simplex[3];
		std::vector<Triangle> polytope = { Triangle(a,b,c), Triangle(a,d,b), Triangle(a,c,d), Triangle(b,d,c) };
		ret.push_back(calculateEPA(collider1, collider2, polytope));

		return ret;
	}

// Private functions
	bool FineCollisionDetector::calculateGJK(
		const Collider& collider1, const Collider& collider2,
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
			if (glm::dot(supportPoint.mV, direction) > 0) {
				// 3.1 Add the point and update the simplex
				simplex.push_back(supportPoint);
				flag = !doSimplex(simplex, direction);
			}
			else {
				// 3.2 There is no collision, exit without finishing the simplex
				return false;
			}
		}

		return true;
	}


	Contact FineCollisionDetector::calculateEPA(
		const Collider& collider1, const Collider& collider2,
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
			if (closestFDist - closestFDist2 <= TOLERANCE) {
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

		// 8. Calculate the global coordinates of the contact from the
		// barycenter coordinates of the point
		glm::vec3 cp;
		cp.x = baryPosition.x * closestF->mA.mP1.x + baryPosition.y * closestF->mB.mP1.x + baryPosition.z * closestF->mC.mP1.x;
		cp.y = baryPosition.x * closestF->mA.mP1.y + baryPosition.y * closestF->mB.mP1.y + baryPosition.z * closestF->mC.mP1.y;
		cp.z = baryPosition.x * closestF->mA.mP1.z + baryPosition.y * closestF->mB.mP1.z + baryPosition.z * closestF->mC.mP1.z;

		return Contact(closestFDist, cp, -closestF->mNormal);
	}


	FineCollisionDetector::SupportPoint FineCollisionDetector::getSupportPoint(
		const Collider& collider1, const Collider& collider2,
		const glm::vec3& direction
	) const
	{
		glm::vec3 p1 = collider1.getFurthestPointInDirection(direction);
		glm::vec3 p2 = collider2.getFurthestPointInDirection(-direction);

		return SupportPoint(p1 - p2, p1, p2);
	}


	bool FineCollisionDetector::doSimplex(
		std::vector<SupportPoint>& simplex,
		glm::vec3& direction
	) const
	{
		assert(!simplex.empty() && "The simplex has to have at least one initial point");

		bool ret;
		switch (simplex.size() - 1) {
			case 0:
				ret = doSimplex0D(simplex, direction);
				break;
			case 1:
				ret = doSimplex1D(simplex, direction);
				break;
			case 2:
				ret = doSimplex2D(simplex, direction);
				break;
			case 3:
				ret = doSimplex3D(simplex, direction);
				break;
			default:
				ret = true;
		}

		return ret;
	}


	bool FineCollisionDetector::doSimplex0D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& direction
	) const
	{
		SupportPoint a = simplex[0];
		simplex = { a };
		direction = -a.mV;

		return false;
	}


	bool FineCollisionDetector::doSimplex1D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& direction
	) const
	{
		SupportPoint b = simplex[0], a = simplex[1];
		glm::vec3 ab = b.mV - a.mV, ao = -a.mV;

		if (glm::dot(ab, ao) > 0) {
			// The origin is between b and a
			simplex = { b, a };
			direction = glm::cross(glm::cross(ab, ao), ab);
		}
		else {
			// Discard b and do the same than with 0 dimensions
			simplex = { a };
			direction = ao;
		}

		return false;
	}


	bool FineCollisionDetector::doSimplex2D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& direction
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
			ret = doSimplex1D(simplex, direction);
		}
		else {
			if (glm::dot(glm::cross(abc, ac), ao) > 0) {
				// Origin outside the triangle from the ac edge
				// Discard b point and test the edge in 1 dimension
				simplex = { c, a };
				ret = doSimplex1D(simplex, direction);
			}
			else {
				// Inside the triangle in 2D
				// Check if the origin is above or below the triangle
				if (glm::dot(abc, ao) > 0) {
					simplex = { c, b, a };
					direction = abc;
				}
				else {
					simplex = { b, c, a };
					direction = -abc;
				}

				ret = false;
			}
		}

		return ret;
	}


	bool FineCollisionDetector::doSimplex3D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& direction
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
			ret = doSimplex2D(simplex, direction);
		}
		else {
			if (glm::dot(acd, ao) > 0) {
				// Origin outside the tetrahedron from the acd face
				// Discard b and check the triangle in 2 dimensions
				simplex = { d, c, a };
				ret = doSimplex2D(simplex, direction);
			}
			else {
				if (glm::dot(adb, ao) > 0) {
					// Origin outside the tetrahedron from the adb face
					// Discard c and check the triangle in 2 dimensions
					simplex = { b, d, a };
					ret = doSimplex2D(simplex, direction);
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
