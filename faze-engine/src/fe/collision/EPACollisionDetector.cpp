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
		int iClosestFace = expandPolytope(collider1, collider2, polytope);
		if (iClosestFace < 0) {
			return false;
		}

		// 4. Fill the Contact data with the cloest face of the Polytope
		fillContactData(polytope, iClosestFace, ret);

		return true;
	}

// Private functions
	int EPACollisionDetector::expandPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		const HalfEdgeMesh& meshData = polytope.getMesh();
		auto compareDistances = [&](int iF1, int iF2) {
			return (polytope.getDistanceData(iF1).distance < polytope.getDistanceData(iF2).distance);
		};

		std::vector<int> closestFaces;
		for (auto itFace = meshData.getFacesVector().begin(); itFace != meshData.getFacesVector().end(); ++itFace) {
			closestFaces.insert(
				std::upper_bound(closestFaces.begin(), closestFaces.end(), itFace.getIndex(), compareDistances),
				itFace.getIndex()
			);
			closestFaces.push_back(itFace.getIndex());
		}

		float closestSeparation = std::numeric_limits<float>::max();
		while (!closestFaces.empty()) {
			// 1. Get the next closest face to the origin
			int iClosestFace = closestFaces.front();
			FaceDistanceData faceDistance = polytope.getDistanceData(iClosestFace);

			// 2. Add a new SupportPoint along the closest point direction
			int iSp = polytope.addVertex(SupportPoint(collider1, collider2, faceDistance.closestPoint));
			SupportPoint sp = polytope.getSupportPoint(iSp);

			// 3. If the origin is in the triangle or the new SupportPoint is
			// no further to the current triangle than mMinFThreshold then
			// we've found the closest PolytopeFace
			float spSeparation = glm::dot(sp.getCSOPosition(), faceDistance.closestPoint / faceDistance.distance);
			closestSeparation = std::min(closestSeparation, spSeparation);
			if ((faceDistance.distance == 0)
				|| (closestSeparation - faceDistance.distance < mMinFThreshold)
			) {
				return iClosestFace;
			}

			// 4. Calculate the horizon HEEdges and HEFaces to remove from the
			// current eyePoint perspective
			std::vector<int> horizon, facesToRemove;
			std::tie(horizon, facesToRemove) = calculateHorizon(
				sp.getCSOPosition(), iClosestFace,
				meshData, polytope.getNormalsVector()
			);

			// 5. Remove all the faces that can be seen from the new
			// SupportPoint
			for (int iFaceToRemove : facesToRemove) {
				closestFaces.erase(std::find(closestFaces.begin(), closestFaces.end(), iFaceToRemove));
				polytope.removeFace(iFaceToRemove);
			}

			// 5. Add new faces to the Polytope by connecting the edges of the
			// horizon to the new SupportPoint
			for (int iHorizonEdge : horizon) {
				const HEEdge& currentEdge	= meshData.getEdge(iHorizonEdge);
				const HEEdge& oppositeEdge	= meshData.getEdge(currentEdge.oppositeEdge);

				// Create the new HEFace
				int iV0 = oppositeEdge.vertex, iV1 = currentEdge.vertex;
				int iNewFace = polytope.addFace({ iV0, iV1, iSp });

				// Add the face to the faceQueue
				closestFaces.insert(
					std::upper_bound(closestFaces.begin(), closestFaces.end(), iNewFace, compareDistances),
					iNewFace
				);
			}
		}

		return -1;
	}


	void EPACollisionDetector::fillContactData(
		const Polytope& polytope, int iClosestFace, Contact& ret
	) const
	{
		const HalfEdgeMesh& meshData = polytope.getMesh();
		FaceDistanceData faceDistance = polytope.getDistanceData(iClosestFace);
		glm::vec3 faceNormal = polytope.getNormalsVector().find(iClosestFace)->second;

		const HEEdge& edge1		= meshData.getEdge(meshData.getFace(iClosestFace).edge);
		const SupportPoint& sp1	= polytope.getSupportPoint(edge1.vertex);

		const HEEdge& edge2		= meshData.getEdge(edge1.nextEdge);
		const SupportPoint& sp2	= polytope.getSupportPoint(edge2.vertex);

		const HEEdge& edge3		= meshData.getEdge(edge2.nextEdge);
		const SupportPoint& sp3	= polytope.getSupportPoint(edge3.vertex);

		const glm::vec3& originBarycentricCoords = faceDistance.closestPointBarycentricCoords;
		ret.mPenetration = faceDistance.distance;
		ret.mNormal = -faceNormal;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.mWorldPosition[i][j] = originBarycentricCoords.x * sp1.getWorldPosition(i)[j]
					+ originBarycentricCoords.y * sp2.getWorldPosition(i)[j]
					+ originBarycentricCoords.z * sp3.getWorldPosition(i)[j];
				ret.mLocalPosition[i][j] = originBarycentricCoords.x * sp1.getLocalPosition(i)[j]
					+ originBarycentricCoords.y * sp2.getLocalPosition(i)[j]
					+ originBarycentricCoords.z * sp3.getLocalPosition(i)[j];
			}
		}
	}

}}
