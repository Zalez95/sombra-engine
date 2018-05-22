#include <tuple>
#include <limits>
#include <cassert>
#include <algorithm>
#include "fe/collision/Contact.h"
#include "fe/collision/ConvexCollider.h"
#include "fe/collision/FineCollisionDetector.h"
#include "Polytope.h"

namespace fe { namespace collision {

	EPACollisionDetector::EPACollisionDetector(
		float minFDifference, float projectionPrecision
	) : mMinFThreshold(minFDifference),
		mProjectionPrecision(projectionPrecision)
	{
		assert(minFDifference >= 0 && "The minimum face difference must be at least 0");
		assert(projectionPrecision >= 0 && "The precision of the projected points must be at least 0");
	}


	bool EPACollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex, Contact& ret
	) const
	{
		// 1. Calculate the closest face to the origin
		Triangle* closestF;
		float closestFDist;
		std::tie(closestF, closestFDist) = calculateEPA(collider1, collider2, simplex);
		if (closestF == nullptr) {
			return false;
		}

		// 2. Project the origin onto the closest face and calculate its
		// barycentric coordinates
		glm::vec3 originBarycentricCoords,
			a = closestF->ab.p1->getCSOPosition(),
			b = closestF->bc.p1->getCSOPosition(),
			c = closestF->ca.p1->getCSOPosition();
		if (!projectPointOnTriangle(glm::vec3(0.0f), { a, b, c }, mProjectionPrecision, originBarycentricCoords)) {
			return false;
		}

		// 3. Fill the Contact data with the face normal, it's distance to the
		// origin and the barycentric coordinates of the origin onto the face
		ret.mPenetration = closestFDist;
		ret.mNormal = closestF->normal;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.mWorldPosition[i][j] = originBarycentricCoords.x * closestF->ab.p1->getWorldPosition(i)[j]
					+ originBarycentricCoords.y * closestF->bc.p1->getWorldPosition(i)[j]
					+ originBarycentricCoords.z * closestF->ca.p1->getWorldPosition(i)[j];
				ret.mLocalPosition[i][j] = originBarycentricCoords.x * closestF->ab.p1->getLocalPosition(i)[j]
					+ originBarycentricCoords.y * closestF->bc.p1->getLocalPosition(i)[j]
					+ originBarycentricCoords.z * closestF->ca.p1->getLocalPosition(i)[j];
			}
		}

		return true;
	}

// Private functions
	std::pair<Triangle*, float> EPACollisionDetector::calculateEPA(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) const
	{
		// 1. Create the initial polytope to expand from the simplex points
		Polytope polytope(collider1, collider2, simplex);
		if (polytope.faces.size() < 4) {
			return std::make_pair(nullptr, -1);
		}

		std::vector<Triangle>::iterator closestFIt;
		float closestFDist = std::numeric_limits<float>::max();
		for (int nIterations = 0; nIterations < sMaxIterations; ++nIterations) {
			// 2. Calculate the closest face to the origin of the polytope
			std::tie(closestFIt, closestFDist) = getClosestFaceToOrigin(polytope);

			// 3. Add a new support point along the face normal
			polytope.vertices.emplace_back(collider1, collider2, closestFIt->normal);
			SupportPoint* sp = &polytope.vertices.back();

			// 4. If the distance to the origin of the new SupportPoint is no
			// further to the origin than the current closest face by
			// mMinFThreshold, then we have found the closest face
			float distanceSPFace = glm::dot(sp->getCSOPosition(), closestFIt->normal) - closestFDist;
			if (distanceSPFace <= mMinFThreshold) {
				return std::make_pair(&(*closestFIt), closestFDist);
			}

			// 5. Delete the faces that can be seen from the new point and get
			// the edges of the created hole in the Polytope
			std::vector<Edge> holeEdges = { closestFIt->ab, closestFIt->bc, closestFIt->ca };
			polytope.faces.erase(closestFIt);
			for (auto it = polytope.faces.begin(); it != polytope.faces.end();) {
				if (glm::dot(it->normal, sp->getCSOPosition()) > 0) {
					appendEdge(it->ab, holeEdges);
					appendEdge(it->bc, holeEdges);
					appendEdge(it->ca, holeEdges);
					it = polytope.faces.erase(it);
				}
				else {
					++it;
				}
			}

			// 6. Add new faces connecting the edges of the hole to the support
			// point
			for (const Edge& e : holeEdges) {
				polytope.faces.emplace_back(sp, e.p1, e.p2);
			}
		}

		return std::make_pair(nullptr, -1);
	}


	EPACollisionDetector::ClosestFace EPACollisionDetector::getClosestFaceToOrigin(
		Polytope& polytope
	) const
	{
		std::vector<Triangle>::iterator closestFIt = polytope.faces.end();
		float closestFDist = std::numeric_limits<float>::max();
		for (auto it = polytope.faces.begin(); it != polytope.faces.end(); ++it) {
			float distance = std::abs(glm::dot(it->normal, it->ab.p1->getCSOPosition()));
			if (distance < closestFDist) {
				closestFIt		= it;
				closestFDist	= distance;
			}
		}

		return std::make_pair(closestFIt, closestFDist);
	}


	void EPACollisionDetector::appendEdge(const Edge& e, std::vector<Edge>& edgeVector) const
	{
		for (auto itEdge = edgeVector.begin(); itEdge != edgeVector.end(); ++itEdge) {
			if (e == *itEdge) {
				std::swap(*itEdge, edgeVector.back());
				edgeVector.pop_back();
				return;
			}
		}

		edgeVector.push_back(e);
	}

}}
