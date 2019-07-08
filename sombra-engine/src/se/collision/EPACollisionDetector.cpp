#include <limits>
#include <cassert>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/collision/Contact.h"
#include "se/collision/ConvexCollider.h"
#include "se/collision/HalfEdgeMeshExt.h"
#include "se/collision/FineCollisionDetector.h"
#include "se/utils/FixedVector.h"
#include "Polytope.h"

namespace se::collision {

	EPACollisionDetector::EPACollisionDetector(
		float minFDifference, float projectionPrecision
	) : mMinFThreshold(minFDifference),
		mProjectionPrecision(projectionPrecision)
	{
		assert(minFDifference >= 0.0f && "The minimum face difference must be at least 0");
		assert(projectionPrecision >= 0.0f && "The precision of the projected points must be at least 0");
	}


	bool EPACollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		InitialSimplex& simplex, Contact& ret
	) const
	{
		bool contactUpdated = false;

		if (simplex.size() == 1) {
			// The simplex's only point is the origin
			ret.penetration = 0.0f;
			ret.normal = glm::vec3(0.0f);
			for (int i = 0; i < 2; ++i) {
				ret.worldPosition[i] = simplex[0].getWorldPosition(i);
				ret.localPosition[i] = simplex[0].getLocalPosition(i);
			}
			contactUpdated = true;
		}
		else if (simplex.size() > 1) {
			// Create the initial polytope to expand from the simplex points
			Polytope polytope = createInitialPolytope(collider1, collider2, simplex);

			// Calculate the closest face to the origin
			int iClosestFace = expandPolytope(collider1, collider2, polytope);
			if (iClosestFace >= 0) {
				// Fill the Contact data with the closest face of the Polytope
				fillContactData(polytope, iClosestFace, ret);
				contactUpdated = true;
			}
		}

		return contactUpdated;
	}

// Private functions
	Polytope EPACollisionDetector::createInitialPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		InitialSimplex& simplex
	) const
	{
		if (simplex.size() == 2) {
			tetrahedronFromEdge(collider1, collider2, simplex);
		}
		else if (simplex.size() == 3) {
			tetrahedronFromTriangle(collider1, collider2, simplex);
		}

		return Polytope(simplex, mProjectionPrecision);
	}


	void EPACollisionDetector::tetrahedronFromEdge(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		InitialSimplex& simplex
	) const
	{
		InitialSimplex vertices;
		glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();

		// 1. Find the closest coordinate axis to being orthonormal to the
		// direction v01
		glm::vec3 vAxis(0.0f);
		auto absCompare = [](float f1, float f2) { return std::abs(f1) < std::abs(f2); };
		int iAxis = std::distance(&v01.x, std::min_element(&v01.x, &v01.x + 3, absCompare));
		vAxis[iAxis] = 1.0f;

		// 2. Calculate an orthonormal vector to the vector v01 with vAxis
		glm::vec3 vNormal = glm::cross(v01, vAxis);

		// 3. Calculate 3 new points around the vector v01 by rotating vNormal
		// 2*pi/3 radians around v01
		glm::mat3 rotate2Pi3 = glm::mat3(glm::rotate(glm::mat4(1.0f), 2.0f * glm::pi<float>() / 3.0f, v01));
		glm::vec3 searchDir = vNormal;
		for (int i = 0; i < 3; ++i) {
			vertices.emplace_back(collider1, collider2, searchDir);
			searchDir = rotate2Pi3 * searchDir;
		}

		// 4. The fourth point of the polytope must be either simplex[0] or
		// simplex[1], we select the one that creates a tetrahedron with the
		// origin inside
		glm::vec3 a = vertices[0].getCSOPosition(), b = vertices[1].getCSOPosition(), c = vertices[2].getCSOPosition();
		glm::vec3 tNormal = glm::cross(b - a, c - a);
		if ((glm::dot(a, tNormal) > 0.0f) && (glm::dot(simplex[0].getCSOPosition(), tNormal) > 0.0f)) {
			vertices.push_back(simplex[1]);
		}
		else if ((glm::dot(a, tNormal) < 0.0f) && (glm::dot(simplex[0].getCSOPosition(), tNormal) < 0.0f)) {
			vertices.push_back(simplex[1]);
		}
		else {
			vertices.push_back(simplex[0]);
		}

		simplex = vertices;
	}


	void EPACollisionDetector::tetrahedronFromTriangle(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		InitialSimplex& simplex
	) const
	{
		// Search two support points with the simplex's triangle normal
		glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();
		glm::vec3 v02 = simplex[2].getCSOPosition() - simplex[0].getCSOPosition();
		glm::vec3 tNormal = glm::cross(v01, v02);

		SupportPoint sp1(collider1, collider2, tNormal);
		SupportPoint sp2(collider1, collider2,-tNormal);

		if (isOriginInsideTetrahedron({
				simplex[0].getCSOPosition(), simplex[1].getCSOPosition(),
				sp1.getCSOPosition(), sp2.getCSOPosition()
			})
		) {
			simplex = { simplex[0], simplex[1], sp1, sp2 };
		}
		else if (isOriginInsideTetrahedron({
				simplex[1].getCSOPosition(), simplex[2].getCSOPosition(),
				sp1.getCSOPosition(), sp2.getCSOPosition()
			})
		) {
			simplex = { simplex[1], simplex[2], sp1, sp2 };
		}
		else {
			simplex = { simplex[2], simplex[0], sp1, sp2 };
		}
	}


	bool EPACollisionDetector::isOriginInsideTetrahedron(
		const utils::FixedVector<glm::vec3, 4>& vertices
	) const
	{
		if (vertices.size() < 4) { return false; }

		for (int i = 0; i < 4; ++i) {
			int iV1 = i % 3, iV2 = (i + 1) % 3, iV3 = (i + 2) % 3, iV4 = (i + 3) % 3;

			glm::vec3 v1v2 = vertices[iV2] - vertices[iV1];
			glm::vec3 v1v3 = vertices[iV3] - vertices[iV1];
			glm::vec3 tNormal = glm::normalize(glm::cross(v1v2, v1v3));
			if ((glm::dot(tNormal, vertices[iV1]) > -mProjectionPrecision)
				== (glm::dot(tNormal, vertices[iV4]) > -mProjectionPrecision)
			) {
				return false;
			}
		}

		return true;
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

		// Store the HEFace indices in a vector ordered by their distance to
		// the origin
		std::vector<int> facesByDistance;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			if (polytope.getDistanceData(itFace.getIndex()).inside) {
				facesByDistance.insert(
					std::lower_bound(facesByDistance.begin(), facesByDistance.end(), itFace.getIndex(), compareDistances),
					itFace.getIndex()
				);
			}
		}

		// Check if there is no polytope face with its closest point to the
		// origin inside
		if (facesByDistance.empty()) {
			return -1;
		}

		// Check if the closest HEFace to the origin is already touching it
		int iCurrentFace = facesByDistance.back();
		facesByDistance.pop_back();
		FaceDistanceData faceDistance = polytope.getDistanceData(iCurrentFace);
		if (faceDistance.distance == 0.0f) {
			return iCurrentFace;
		}

		// Expand the polytope until the closest HEFace is found
		int iClosestFace = -1;
		float closestSeparation = std::numeric_limits<float>::max();
		utils::FixedVector<int, 3> closestFaceIndices;
		std::vector<int> overlappingFaces;
		do {
			// 1. Search a new SupportPoint along the HEFace's closest point direction
			SupportPoint sp(collider1, collider2, faceDistance.closestPoint);

			// 2. Update the closest separation and HEFace
			float currentSeparation = glm::dot(sp.getCSOPosition(), glm::normalize(faceDistance.closestPoint));
			if (currentSeparation < closestSeparation) {
				closestSeparation = currentSeparation;
				iClosestFace = iCurrentFace;
			}

			// 3. If the current HEFace is closer to the origin than the closest
			// one then we expand the polytope
			if (closestSeparation - faceDistance.distance > mMinFThreshold) {
				// 3.1 Add the SupportPoint to the Polytope
				int iSp = polytope.addVertex(sp);

				// 3.2. Calculate the horizon HEEdges and HEFaces to remove
				// from the current eyePoint perspective
				auto [horizon, facesToRemove] = calculateHorizon(
					meshData, polytope.getNormals(),
					sp.getCSOPosition(), iCurrentFace
				);

				// 3.3. Remove all the HEFaces that can be seen from the new
				// SupportPoint
				for (int iFaceToRemove : facesToRemove) {
					// If we are going to remove the closest HEFace then we
					// store its indices for recoving it later if necessary
					if (iClosestFace == iFaceToRemove) {
						closestFaceIndices.clear();
						getFaceIndices(polytope.getMesh(), iClosestFace, std::back_inserter(closestFaceIndices));
						iClosestFace = -1;
						overlappingFaces.clear();
					}

					polytope.removeFace(iFaceToRemove);
					facesByDistance.erase(
						std::remove(facesByDistance.begin(), facesByDistance.end(), iFaceToRemove),
						facesByDistance.end()
					);
				}

				// 3.4. Add new HEFaces to the Polytope by connecting the
				// HEEdges of the horizon to the new SupportPoint
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

			// 4. Get the next closest HEFace to the origin
			if (!facesByDistance.empty()) {
				iCurrentFace = facesByDistance.back();
				facesByDistance.pop_back();
				faceDistance = polytope.getDistanceData(iCurrentFace);
			}
		}
		while (!facesByDistance.empty() && (closestSeparation - faceDistance.distance > mMinFThreshold));

		// If we removed the closestFace then we have to recover it
		if (iClosestFace < 0) {
			for (int iFace : overlappingFaces) { polytope.removeFace(iFace); }
			iClosestFace = polytope.addFace(closestFaceIndices);
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
		ret.penetration = faceDistance.distance;
		ret.normal = faceNormal;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) {
				ret.worldPosition[i][j] = originBarycentricCoords.x * sp1.getWorldPosition(i)[j]
					+ originBarycentricCoords.y * sp2.getWorldPosition(i)[j]
					+ originBarycentricCoords.z * sp3.getWorldPosition(i)[j];
				ret.localPosition[i][j] = originBarycentricCoords.x * sp1.getLocalPosition(i)[j]
					+ originBarycentricCoords.y * sp2.getLocalPosition(i)[j]
					+ originBarycentricCoords.z * sp3.getLocalPosition(i)[j];
			}
		}
	}

}
