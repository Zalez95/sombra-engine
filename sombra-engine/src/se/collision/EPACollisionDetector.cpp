#include <limits>
#include <cassert>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/collision/Contact.h"
#include "se/collision/ConvexCollider.h"
#include "se/collision/HalfEdgeMeshExt.h"
#include "se/collision/FineCollisionDetector.h"
#include "Polytope.h"

namespace se::collision {

	EPACollisionDetector::EPACollisionDetector(
		float minFDifference, std::size_t maxIterations, float projectionPrecision
	) : mMinFThreshold(minFDifference), mMaxIterations(maxIterations), mProjectionPrecision(projectionPrecision)
	{
		assert(minFDifference >= 0.0f && "The minimum face difference must be at least 0");
		assert(projectionPrecision >= 0.0f && "The precision of the projected points must be at least 0");
	}


	std::pair<bool, Contact> EPACollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Simplex& simplex
	) const
	{
		std::pair<bool, Contact> ret = { false, Contact{} };

		if (simplex.size() == 1) {
			// The simplex's only point is the origin
			ret.second.penetration = 0.0f;
			ret.second.normal = glm::vec3(0.0f);
			for (int i = 0; i < 2; ++i) {
				ret.second.worldPosition[i] = simplex[0].getWorldPosition(i);
				ret.second.localPosition[i] = simplex[0].getLocalPosition(i);
			}
			ret.first = true;
		}
		else if (simplex.size() > 1) {
			// Create the initial polytope to expand from the simplex points
			Polytope polytope = createInitialPolytope(collider1, collider2, simplex);

			// Calculate the closest face to the origin
			int iClosestFace = expandPolytope(collider1, collider2, polytope);
			if (iClosestFace >= 0) {
				// Fill the Contact data with the closest face of the Polytope
				ret.second = calculateContactData(polytope, iClosestFace);
				ret.first = true;
			}
		}

		return ret;
	}

// Private functions
	Polytope EPACollisionDetector::createInitialPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Simplex& simplex
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
		Simplex& simplex
	) const
	{
		Simplex vertices;
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
		Simplex& simplex
	) const
	{
		// Search two support points with the simplex's triangle normal
		glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();
		glm::vec3 v02 = simplex[2].getCSOPosition() - simplex[0].getCSOPosition();
		glm::vec3 tNormal = glm::cross(v01, v02);

		SupportPoint sp1(collider1, collider2, tNormal);
		SupportPoint sp2(collider1, collider2,-tNormal);

		simplex = { simplex[0], simplex[1], sp1, sp2 };
		if (!isOriginInside(simplex, mProjectionPrecision)) {
			simplex = { simplex[1], simplex[2], sp1, sp2 };

			if (!isOriginInside(simplex, mProjectionPrecision)) {
				simplex = { simplex[2], simplex[0], sp1, sp2 };
			}
		}
	}


	int EPACollisionDetector::expandPolytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		Polytope& polytope
	) const
	{
		auto compareDistances = [&](int iF1, int iF2) {
			return (polytope.getDistanceData(iF1).distance > polytope.getDistanceData(iF2).distance);
		};

		// Store the HEFace indices in a vector ordered by their distance to
		// the origin
		const HalfEdgeMesh& meshData = polytope.getMesh();
		std::vector<int> facesByDistance;
		facesByDistance.reserve(meshData.faces.size());
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
		glm::vec3 currentFaceNormal = polytope.getNormal(iCurrentFace);
		FaceDistanceData currentFaceDistance = polytope.getDistanceData(iCurrentFace);
		facesByDistance.pop_back();
		if (currentFaceDistance.distance == 0.0f) {
			return iCurrentFace;
		}

		// Expand the polytope until the closest HEFace is found
		std::size_t iteration = 0;
		int iClosestFace = -1;
		float closestSeparation = std::numeric_limits<float>::max();
		utils::FixedVector<int, 3> closestFaceIndices;
		std::vector<int> overlappingFaces;
		do {
			// 1. Search a new SupportPoint along the HEFace's closest point
			// direction
			SupportPoint sp(collider1, collider2, currentFaceNormal);

			// 2. Update the closest HEFace
			float currentSeparation = glm::dot(sp.getCSOPosition(), currentFaceNormal);
			if (currentSeparation < closestSeparation) {
				iClosestFace = iCurrentFace;
				closestSeparation = currentSeparation;
			}

			// 3. If the current HEFace is closer to the origin than the closest
			// one then we expand the polytope
			if (closestSeparation - currentFaceDistance.distance > mMinFThreshold) {
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

					// Add the HEFace
					int iV0 = oppositeEdge.vertex, iV1 = currentEdge.vertex;
					int iNewFace = polytope.addFace({ iV0, iV1, iSp });

					// Store the HEFace index if we removed the closest HEFace
					if (iClosestFace < 0) {
						overlappingFaces.push_back(iNewFace);
					}

					// Add the HEFace to the facesByDistance vector if its
					// closest point is an internal point
					if (polytope.getDistanceData(iNewFace).inside) {
						facesByDistance.insert(
							std::lower_bound(facesByDistance.begin(), facesByDistance.end(), iNewFace, compareDistances),
							iNewFace
						);
					}
				}
			}

			// Check if there are more HEFaces to evaluate
			if (!facesByDistance.empty()) {
				// Get the next HEFace
				iCurrentFace = facesByDistance.back();
				currentFaceNormal = polytope.getNormal(iCurrentFace);
				currentFaceDistance = polytope.getDistanceData(iCurrentFace);
				facesByDistance.pop_back();
			}

			++iteration;
		}
		while (!facesByDistance.empty()
			&& (closestSeparation - currentFaceDistance.distance > mMinFThreshold)
			&& (iteration < mMaxIterations)
		);

		// If we removed the closest HEFace then we have to recover it
		if (iClosestFace < 0) {
			for (int iFace : overlappingFaces) {
				polytope.removeFace(iFace);
			}
			iClosestFace = polytope.addFace(closestFaceIndices);
		}

		return iClosestFace;
	}


	Contact EPACollisionDetector::calculateContactData(const Polytope& polytope, int iClosestFace) const
	{
		Contact ret;

		const HalfEdgeMesh& meshData = polytope.getMesh();
		glm::vec3 faceNormal = polytope.getNormal(iClosestFace);
		FaceDistanceData faceDistance = polytope.getDistanceData(iClosestFace);

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
			ret.worldPosition[i] = originBarycentricCoords.x * sp1.getWorldPosition(i)
				+ originBarycentricCoords.y * sp2.getWorldPosition(i)
				+ originBarycentricCoords.z * sp3.getWorldPosition(i);
			ret.localPosition[i] = originBarycentricCoords.x * sp1.getLocalPosition(i)
				+ originBarycentricCoords.y * sp2.getLocalPosition(i)
				+ originBarycentricCoords.z * sp3.getLocalPosition(i);
		}

		return ret;
	}

}
