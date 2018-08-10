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
		if (simplex.empty()) {
			return false;
		}
		else if (simplex.size() < 3) {
			calculateDegenerateContact(simplex, ret);
			return true;
		}

		// 2. Create the initial polytope to expand from the simplex points
		Polytope polytope = createInitialPolytope(collider1, collider2, simplex);

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
	void EPACollisionDetector::calculateDegenerateContact(
		const std::vector<SupportPoint>& simplex, Contact& ret
	) const
	{
		int nVertices = simplex.size();
		if (nVertices == 1) {
			ret.mPenetration = 0.0f;
			ret.mNormal = glm::vec3(0.0f);
			for (int i = 0; i < 2; ++i) {
				ret.mWorldPosition[i] = simplex[0].getWorldPosition(i);
				ret.mLocalPosition[i] = simplex[0].getLocalPosition(i);
			}
		}
		else if (nVertices == 2) {
			std::vector<float> distances = {
				glm::length(simplex[0].getCSOPosition()),
				glm::length(simplex[1].getCSOPosition())
			};

			int iClosestSP = (distances[0] > distances[1])? 0 : 1;
			ret.mPenetration = distances[iClosestSP];
			ret.mNormal = glm::normalize( simplex[iClosestSP].getCSOPosition() );
			for (int i = 0; i < 2; ++i) {
				ret.mWorldPosition[i] = simplex[iClosestSP].getWorldPosition(i);
				ret.mLocalPosition[i] = simplex[iClosestSP].getLocalPosition(i);
			}
		}
	}


	Polytope EPACollisionDetector::createInitialPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) const
	{
		if (simplex.size() == 3) {
			// Expand the initial triangle simplex to a tetrahedron
			// Search a support point along the simplex's triangle normal
			glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();
			glm::vec3 v02 = simplex[2].getCSOPosition() - simplex[0].getCSOPosition();
			glm::vec3 tNormal = glm::cross(v01, v02);

			SupportPoint sp(collider1, collider2, tNormal);
			if (glm::dot(sp.getCSOPosition() - simplex[0].getCSOPosition(), tNormal) < 0) {
				// Try the opposite direction
				sp = SupportPoint(collider1, collider2, -tNormal);
			}

			simplex.push_back(sp);
		}
	
		return Polytope({ simplex[0], simplex[1], simplex[2], simplex[3] }, mProjectionPrecision);
	}


	int EPACollisionDetector::expandPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		const HalfEdgeMesh& meshData = polytope.getMesh();
		auto compareDistances = [&](int iF1, int iF2) {
			return (polytope.getDistanceData(iF1).distance > polytope.getDistanceData(iF2).distance);
		};

		// Store the HEFace indices in a vector ordered by their distance
		std::vector<int> facesByDistance;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			facesByDistance.insert(
				std::lower_bound(facesByDistance.begin(), facesByDistance.end(), itFace.getIndex(), compareDistances),
				itFace.getIndex()
			);
		}

		int iClosestFace = -1;
		std::vector<int> closestFaceIndices, overlappingFaces;
		float closestSeparation = std::numeric_limits<float>::max();
		while (!facesByDistance.empty() && (closestSeparation >= mMinFThreshold)) {
			// 1. Get the next closest HEFace to the origin
			int iCurrentFace = facesByDistance.back();
			const glm::vec3& faceNormal = polytope.getNormal(iCurrentFace);
			const FaceDistanceData& faceDistance = polytope.getDistanceData(iCurrentFace);
			facesByDistance.pop_back();

			// 2. Add a new SupportPoint along the HEFace normal
			int iSp = polytope.addVertex(SupportPoint(collider1, collider2, faceNormal));
			SupportPoint sp = polytope.getSupportPoint(iSp);

			// 3. Update the HEFace with the closest separation
			float currentSeparation = glm::dot(sp.getCSOPosition(), faceNormal) - faceDistance.distance;
			if (currentSeparation < closestSeparation) {
				closestSeparation = currentSeparation;
				iClosestFace = iCurrentFace;
			}

			// 4. If the new SupportPoint is further from the current HEFace
			// than mMinFThreshold then we expand the polytope
			if (closestSeparation >= mMinFThreshold) {
				// 4.1. Calculate the horizon HEEdges and HEFaces to remove
				// from the current eyePoint perspective
				std::vector<int> horizon, facesToRemove;
				std::tie(horizon, facesToRemove) = calculateHorizon(
					meshData, polytope.getNormalsMap(),
					sp.getCSOPosition(), iCurrentFace
				);

				// 4.2. Remove all the HEFaces that can be seen from the new
				// SupportPoint
				for (int iFaceToRemove : facesToRemove) {
					// If we are going to remove the closest HEFace then we
					// store its indices for recoving it later if necessary
					if (iClosestFace == iFaceToRemove) {
						closestFaceIndices = getFaceIndices(polytope.getMesh(), iClosestFace);
						iClosestFace = -1;
					}

					polytope.removeFace(iFaceToRemove);
					facesByDistance.erase(
						std::remove(facesByDistance.begin(), facesByDistance.end(), iFaceToRemove),
						facesByDistance.end()
					);
				}

				// 4.3. Add new HEFaces to the Polytope by connecting the
				// HEEdges of the horizon to the new SupportPoint
				if (iClosestFace < 0) { overlappingFaces.clear(); }
				for (int iHorizonEdge : horizon) {
					const HEEdge& currentEdge = meshData.edges[iHorizonEdge];
					const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

					// Create the new HEFace
					int iV0 = oppositeEdge.vertex, iV1 = currentEdge.vertex;
					int iNewFace = polytope.addFace({ iV0, iV1, iSp });

					// Add the HEFace to the facesByDistance vector if its
					// closest point is an internal point
					if (polytope.getDistanceData(iNewFace).inside) {
						facesByDistance.insert(
							std::lower_bound(facesByDistance.begin(), facesByDistance.end(), iNewFace, compareDistances),
							iNewFace
						);
					}

					// Store the HEFace index if we deleted the closest HEFace
					if (iClosestFace < 0) { overlappingFaces.push_back(iNewFace); }
				}
			}
		}

		// If we removed the closestFace then we have to recover it
		if (iClosestFace < 0) {
			for (int iFace : overlappingFaces) { polytope.removeFace(iFace); }
			iClosestFace = polytope.addFace({ closestFaceIndices[0], closestFaceIndices[1], closestFaceIndices[2] });
		}

		return iClosestFace;
	}


	void EPACollisionDetector::fillContactData(
		const Polytope& polytope, int iClosestFace, Contact& ret
	) const
	{
		const HalfEdgeMesh& meshData = polytope.getMesh();
		FaceDistanceData faceDistance = polytope.getDistanceData(iClosestFace);
		glm::vec3 faceNormal = polytope.getNormal(iClosestFace);

		const HEEdge& edge1		= meshData.edges[meshData.faces[iClosestFace].edge];
		const SupportPoint& sp2	= polytope.getSupportPoint(edge1.vertex);

		const HEEdge& edge2		= meshData.edges[edge1.nextEdge];
		const SupportPoint& sp3	= polytope.getSupportPoint(edge2.vertex);

		const HEEdge& edge3		= meshData.edges[edge2.nextEdge];
		const SupportPoint& sp1	= polytope.getSupportPoint(edge3.vertex);

		const glm::vec3& originBarycentricCoords = faceDistance.closestPointBarycentricCoords;
		ret.mPenetration = faceDistance.distance;
		ret.mNormal = faceNormal;
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
