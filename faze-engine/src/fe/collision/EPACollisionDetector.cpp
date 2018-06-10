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
		// 1. Handle the weird initial simplex cases
		if (simplex.empty()) { return false; }
		if (simplex.size() == 1) {	// The simplex point is the Contact point
			ret.mPenetration = 0.0f;
			ret.mNormal = glm::vec3(0.0f);
			for (int i = 0; i < 2; ++i) {
				ret.mWorldPosition[i] = simplex[0].getWorldPosition(i);
				ret.mLocalPosition[i] = simplex[0].getLocalPosition(i);
			}
			return true;
		}

		// 2. Create the initial polytope to expand from the simplex points
		Polytope polytope(collider1, collider2, simplex, mProjectionPrecision);

		// 3. Calculate the closest face to the origin
		if (!expandPolytope(collider1, collider2, polytope)) {
			return false;
		}

		// 4. Fill the Contact data with the cloest face of the Polytope
		fillContactData(polytope, ret);

		return true;
	}

// Private functions
	bool EPACollisionDetector::expandPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		float closestSeparation = std::numeric_limits<float>::max();
		while (!polytope.faces.empty()) {
			// 1. Get the next closest face to the origin
			PolytopeFace& closestFace = polytope.faces.front();
			if (closestFace.obsolete) {
				// 2.1. Remove the current obsolete closest face
				polytope.faces.pop_front();
			}
			else {
				// 2.2. Add a new SupportPoint along the closest point direction
				polytope.vertices.emplace_back(collider1, collider2, closestFace.closestPoint);
				SupportPoint& sp = polytope.vertices.back();
				int iSp = polytope.vertices.size() - 1;

				// 2.3. If the origin is in the triangle or the new SupportPoint
				// is no further to the current triangle than mMinFThreshold
				// then we've found the closest PolytopeFace
				float spSeparation = glm::dot(sp.getCSOPosition(), closestFace.closestPoint / closestFace.distance);
				closestSeparation = std::min(closestSeparation, spSeparation);
				if ((closestFace.distance == 0)
					|| (closestSeparation - closestFace.distance < mMinFThreshold)
				) {
					return true;
				}

				// 2.4. Mark as obsolete all the faces that can be seen
				// from the new SupportPoint and get the edges of the hole that
				// would be created in the Polytope if we remove them
				closestFace.obsolete = true;
				const Triangle& t = closestFace.triangle;
				std::vector<Edge> holeEdges = { t.ab, t.bc, t.ca };
				for (PolytopeFace& currentFace : polytope.faces) {
					if (!currentFace.obsolete
						&& (glm::dot(currentFace.triangle.normal, sp.getCSOPosition()) > 0.0f)
					) {
						currentFace.obsolete = true;
						appendEdge(currentFace.triangle.ab, holeEdges);
						appendEdge(currentFace.triangle.bc, holeEdges);
						appendEdge(currentFace.triangle.ca, holeEdges);
					}
				}

				// 2.5. Add new faces to the Polytope by connecting the edges
				// of the hole to the new SupportPoint
				for (const Edge& e : holeEdges) {
					PolytopeFace nextFace({ iSp, e.p1, e.p2 }, polytope.vertices, mProjectionPrecision);
					if (nextFace.inside) {
						polytope.addFace(nextFace);
					}
				}
			}
		}

		return false;
	}


	void EPACollisionDetector::appendEdge(const Edge& e, std::vector<Edge>& edgeVector) const
	{
		auto itEdge = std::find(edgeVector.begin(), edgeVector.end(), e);
		if (itEdge == edgeVector.end()) {
			edgeVector.push_back(e);
		}
		else {
			std::swap(*itEdge, edgeVector.back());
			edgeVector.pop_back();
		}
	}


	void EPACollisionDetector::fillContactData(const Polytope& polytope, Contact& ret) const
	{
		const PolytopeFace& closestFace = polytope.faces.front();
		const SupportPoint& p1 = polytope.vertices[closestFace.triangle.ab.p1];
		const SupportPoint& p2 = polytope.vertices[closestFace.triangle.bc.p1];
		const SupportPoint& p3 = polytope.vertices[closestFace.triangle.ca.p1];
		const glm::vec3& originBarycentricCoords = closestFace.closestPointBarycentricCoords;
		ret.mPenetration = closestFace.distance;
		ret.mNormal = -closestFace.triangle.normal;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.mWorldPosition[i][j] = originBarycentricCoords.x * p1.getWorldPosition(i)[j]
					+ originBarycentricCoords.y * p2.getWorldPosition(i)[j]
					+ originBarycentricCoords.z * p3.getWorldPosition(i)[j];
				ret.mLocalPosition[i][j] = originBarycentricCoords.x * p1.getLocalPosition(i)[j]
					+ originBarycentricCoords.y * p2.getLocalPosition(i)[j]
					+ originBarycentricCoords.z * p3.getLocalPosition(i)[j];
			}
		}
	}

}}
