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
		// 1. Handle the weird simplex cases
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

		// 2. Calculate the closest face to the origin
		PolytopeFace* closestFace = calculateEPA(collider1, collider2, simplex);
		if (!closestFace) {
			return false;
		}

		// 3. Fill the Contact data with the face normal, it's distance to the
		// origin and the barycentric coordinates of the origin onto the face
		Triangle& t = closestFace->triangle;
		glm::vec3 originBarycentricCoords = closestFace->closestPointBarycentricCoords;
		ret.mPenetration = closestFace->distance;
		ret.mNormal = t.normal;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.mWorldPosition[i][j] = originBarycentricCoords.x * t.ab.p1->getWorldPosition(i)[j]
					+ originBarycentricCoords.y * t.bc.p1->getWorldPosition(i)[j]
					+ originBarycentricCoords.z * t.ca.p1->getWorldPosition(i)[j];
				ret.mLocalPosition[i][j] = originBarycentricCoords.x * t.ab.p1->getLocalPosition(i)[j]
					+ originBarycentricCoords.y * t.bc.p1->getLocalPosition(i)[j]
					+ originBarycentricCoords.z * t.ca.p1->getLocalPosition(i)[j];
			}
		}

		return true;
	}

// Private functions
	PolytopeFace* EPACollisionDetector::calculateEPA(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) const
	{
		// 1. Create the initial polytope to expand from the simplex points
		Polytope polytope(collider1, collider2, simplex, mProjectionPrecision);
		if (polytope.faces.empty()) {
			return nullptr;
		}

		float upperBound = std::numeric_limits<float>::max();
		PolytopeFace* closestFace = &polytope.faces.front();
		do {
			if (!closestFace->obsolete) {
				// 2. Add a new support point along the closest point direction
				polytope.vertices.emplace_back(collider1, collider2, closestFace->closestPoint);
				SupportPoint* sp = &polytope.vertices.back();

				// 3. If the new point is further from the origin than the
				// closest point in the current triangle we update our current
				// upper bound
				float currentBound = glm::dot(sp->getCSOPosition(), closestFace->closestPoint / closestFace->distance);
				upperBound = std::min(upperBound, currentBound);

				if (closestFace->distance - upperBound < mMinFThreshold) {
					// 5. Mark as obsolete all the faces that can be seen from
					// the new point and get the edges of the created hole that
					// would be created in the Polytope if we remove them
					closestFace->obsolete = true;
					std::vector<Edge> holeEdges = {
						closestFace->triangle.ab,
						closestFace->triangle.bc,
						closestFace->triangle.ca
					};
					for (PolytopeFace& currentFace : polytope.faces) {
						if (glm::dot(currentFace.triangle.normal, sp->getCSOPosition()) > 0) {
							appendEdge(currentFace.triangle.ab, holeEdges);
							appendEdge(currentFace.triangle.bc, holeEdges);
							appendEdge(currentFace.triangle.ca, holeEdges);
							currentFace.obsolete = true;
						}
					}

					// 6. Add new faces to the polytope by connecting the edges
					// of the hole to the new support point
					for (const Edge& e : holeEdges) {
						PolytopeFace nextFace(sp, e.p1, e.p2, mProjectionPrecision);
						if (nextFace.inside) {
							polytope.addFace(nextFace);
						}
					}
				}
			}

			// 7. Remove the current obsolete face and get the next one
			polytope.faces.pop_front();
			closestFace = &polytope.faces.front();
		}
		while((polytope.faces.size() != 1) && (closestFace->distance - upperBound < mMinFThreshold));

		return closestFace;
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
