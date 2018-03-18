#include <tuple>
#include <limits>
#include <algorithm>
#include "fe/collision/Contact.h"
#include "fe/collision/ConvexCollider.h"
#include "fe/collision/FineCollisionDetector.h"

namespace fe { namespace collision {

	Contact EPACollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		// 1. Calculate the closest face to the origin
		Triangle* closestF;
		float closestFDist;
		std::tie(closestF, closestFDist) = expandPolytope(collider1, collider2, polytope);
		Contact ret(closestFDist, closestF->normal);

		// 2. Project the origin onto the closest triangle and get its
		// barycentric coordinates
		std::array<glm::vec3, 3> triangle = {
			closestF->ab.p1->getCSOPosition(),
			closestF->bc.p1->getCSOPosition(),
			closestF->ca.p1->getCSOPosition()
		};
		glm::vec3 baryCoordinates;
		/* TODO: bool success = */projectPointOnTriangle(glm::vec3(0.0f), triangle, baryCoordinates);

		// 3. Calculate the local and world coordinates of the contact
		// from the barycenter coordinates of the point
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.mWorldPos[i][j] = baryCoordinates.x * closestF->ab.p1->getWorldPosition(i)[j]
									+ baryCoordinates.y * closestF->bc.p1->getWorldPosition(i)[j]
									+ baryCoordinates.z * closestF->ca.p1->getWorldPosition(i)[j];
				ret.mLocalPos[i][j] = baryCoordinates.x * closestF->ab.p1->getLocalPosition(i)[j]
									+ baryCoordinates.y * closestF->bc.p1->getLocalPosition(i)[j]
									+ baryCoordinates.z * closestF->ca.p1->getLocalPosition(i)[j];
			}
		}

		return ret;
	}

// Private functions		
	std::pair<Triangle*, float> EPACollisionDetector::expandPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		Triangle* closestF = nullptr;
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
				closestF		= &(*closestF2It);
				closestFDist	= closestF2Dist;
				break;
			}

			// 3. Add a new support point along the new closest face normal
			polytope.vertices.emplace_back(collider1, collider2, closestF2It->normal);
			SupportPoint* sp = &polytope.vertices.back();

			// 4. Delete the faces that can be seen from the new point and get
			// the edges of the created hole
			std::list<Edge> holeEdges = { closestF2It->ab, closestF2It->bc, closestF2It->ca };
			polytope.faces.erase(closestF2It);
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

			// 5. Add new faces connecting the edges of the hole to the support point
			for (const Edge& e : holeEdges) {
				polytope.faces.emplace_back(sp, e.p1, e.p2);
			}

			closestFDist = closestF2Dist;
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
		const glm::vec3& point,
		const std::array<glm::vec3, 3>& triangle,
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
