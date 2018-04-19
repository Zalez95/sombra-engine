#include <tuple>
#include <limits>
#include <cassert>
#include <algorithm>
#include "fe/collision/Contact.h"
#include "fe/collision/ConvexCollider.h"
#include "fe/collision/FineCollisionDetector.h"

namespace fe { namespace collision {

	EPACollisionDetector::EPACollisionDetector(
		float minFDifference, float projectionPrecision
	) : mMinFDifference(minFDifference),
		mProjectionPrecision(projectionPrecision)
	{
		assert(minFDifference >= 0 && "The minimum face difference must at least 0");
		assert(projectionPrecision >= 0 && "The precision of the projected points must be at least 0");
	}


	bool EPACollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope, Contact& ret
	) const
	{
		if (polytope.faces.size() < 4) {
			return false;
		}

		// 1. Calculate the closest face to the origin
		Triangle closestF;
		float closestFDist;
		std::tie(closestF, closestFDist) = calculateEPA(collider1, collider2, polytope);

		// 2. Project the origin onto the closest face and calculate its
		// barycentric coordinates
		glm::vec3 originBarycentricCoords,
			a = closestF.ab.p1->getCSOPosition(),
			b = closestF.bc.p1->getCSOPosition(),
			c = closestF.ca.p1->getCSOPosition();
		if (!projectPointOnTriangle(glm::vec3(0.0f), { a, b, c }, originBarycentricCoords)) {
			return false;
		}

		// 3. Fill the Contact data with the face normal, it's distance to the
		// origin and the barycentric coordinates of the origin onto the face
		ret.mPenetration = closestFDist;
		ret.mNormal = closestF.normal;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.mWorldPos[i][j] = originBarycentricCoords.x * closestF.ab.p1->getWorldPosition(i)[j]
									+ originBarycentricCoords.y * closestF.bc.p1->getWorldPosition(i)[j]
									+ originBarycentricCoords.z * closestF.ca.p1->getWorldPosition(i)[j];
				ret.mLocalPos[i][j] = originBarycentricCoords.x * closestF.ab.p1->getLocalPosition(i)[j]
									+ originBarycentricCoords.y * closestF.bc.p1->getLocalPosition(i)[j]
									+ originBarycentricCoords.z * closestF.ca.p1->getLocalPosition(i)[j];
			}
		}

		return true;
	}

// Private functions		
	std::pair<Triangle, float> EPACollisionDetector::calculateEPA(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		Triangle closestF;
		float closestFDist = std::numeric_limits<float>::max();
		while (true) {
			// 1. Calculate the closest face to the origin of the polytope
			std::list<Triangle>::iterator closestF2It;
			float closestF2Dist;
			std::tie(closestF2It, closestF2Dist) = getClosestFaceToOrigin(polytope);

			// 2. If the difference of distance to the origin between the
			// current closest face and last one is smaller than
			// sMinFDifference we have found the closest face
			if (closestFDist - closestF2Dist <= mMinFDifference) {
				if (closestF2Dist < closestFDist) {
					closestF = *closestF2It;
					closestFDist = closestF2Dist;
				}
				break;
			}

			// 3. Update the closest face value
			closestF = *closestF2It;
			closestFDist = closestF2Dist;

			// 4. Expand the polytope along the closest face normal
			expandPolytope(collider1, collider2, polytope, closestF2It);
		}

		return std::make_pair(closestF, closestFDist);
	}


	std::pair<std::list<Triangle>::iterator, float> EPACollisionDetector::getClosestFaceToOrigin(
		Polytope& polytope
	) const
	{
		std::list<Triangle>::iterator closestFIt = polytope.faces.end();
		float closestFDist = std::numeric_limits<float>::max();
		for (auto it = polytope.faces.begin(); it != polytope.faces.end(); ++it) {
			float distance = glm::dot(it->normal, it->ab.p1->getCSOPosition());
			if (distance < closestFDist) {
				closestFIt		= it;
				closestFDist	= distance;
			}
		}

		return std::make_pair(closestFIt, closestFDist);
	}


	void EPACollisionDetector::expandPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope,
		std::list<Triangle>::iterator itFace
	) const
	{
		// 1. Add a new support point along the face normal
		polytope.vertices.emplace_back(collider1, collider2, itFace->normal);
		SupportPoint* sp = &polytope.vertices.back();

		// 2. Delete the faces that can be seen from the new point and get
		// the edges of the created hole in the Polytope
		std::list<Edge> holeEdges = { itFace->ab, itFace->bc, itFace->ca };
		polytope.faces.erase(itFace);
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

		// 3. Add new faces connecting the edges of the hole to the support
		// point
		for (const Edge& e : holeEdges) {
			polytope.faces.emplace_back(sp, e.p1, e.p2);
		}
	}


	void EPACollisionDetector::appendEdge(const Edge& e, std::list<Edge>& edgeList) const
	{
		for (auto itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge) {
			if ((e.p1 == itEdge->p2) && (e.p2 == itEdge->p1)) {
				edgeList.erase(itEdge);
				return;
			}
		}

		edgeList.push_back(e);
	}


	bool EPACollisionDetector::projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		glm::vec3& projectedPoint
	) const
	{
		glm::vec3 u = triangle[1] - triangle[0], v = triangle[2] - triangle[0],
			w = point - triangle[0], n = glm::cross(u, v);

		float gamma	= glm::dot(glm::cross(u, w), n) / glm::dot(n, n);
		float beta	= glm::dot(glm::cross(w, v), n) / glm::dot(n, n);
		float alpha	= 1 - gamma - beta;

		if ((0.0f - mProjectionPrecision <= gamma) && (gamma <= 1.0f + mProjectionPrecision)
			&& (0.0f - mProjectionPrecision <= beta) && (beta <= 1.0f + mProjectionPrecision)
			&& (0.0f - mProjectionPrecision <= alpha) && (alpha <= 1.0f + mProjectionPrecision)
		) {
			projectedPoint = glm::vec3(gamma, beta, alpha);
			return true;
		}

		return false;
	}

}}
