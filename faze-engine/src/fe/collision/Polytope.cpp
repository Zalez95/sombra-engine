#include <exception>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Polytope.h"
#include "Geometry.h"

namespace fe { namespace collision {

	Polytope::Polytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const std::vector<SupportPoint>& simplex, float precision
	) : mPrecision(precision)
	{
		switch(simplex.size()) {
			case 0:
			case 1:
				return;		// Don't calculate the initial tetrahedron polytope
			case 2:
				tetrahedronFromEdge(collider1, collider2, simplex);
				break;
			case 3:
				tetrahedronFromTriangle(collider1, collider2, simplex);
				break;
			default: {
				addVertex(simplex[0]);
				addVertex(simplex[1]);
				addVertex(simplex[2]);
				addVertex(simplex[3]);
				createTetrahedronFaces();
			} break;
		}
	}


	const SupportPoint& Polytope::getSupportPoint(int iVertex) const
	{
		auto itSupportPoint = mVertexSupportPoints.find(iVertex);
		if (itSupportPoint != mVertexSupportPoints.end()) {
			return itSupportPoint->second;
		}
		else {
			throw std::runtime_error("HEVertex not found in Polytope");
		}
	}


	const FaceDistanceData& Polytope::getDistanceData(int iFace) const
	{
		auto itFaceDistance = mFaceDistances.find(iFace);
		if (itFaceDistance != mFaceDistances.end()) {
			return itFaceDistance->second;
		}
		else {
			throw std::runtime_error("HEFace not found in Polytope");
		}
	}


	int Polytope::addVertex(const SupportPoint& sp)
	{
		int iVertex = mMesh.addVertex(sp.getCSOPosition());
		mVertexSupportPoints.emplace(iVertex, sp);

		return iVertex;
	}


	int Polytope::addFace(const std::vector<int>& faceIndices)
	{
		int iFace = mMesh.addFace({ faceIndices[0], faceIndices[1], faceIndices[2] });

		// Add the normal of the HEFace to mFaceNormals
		mFaceNormals[iFace] = calculateFaceNormal(iFace, mMesh);

		// Add the distance data of the HEFace to mFaceDistances
		const glm::vec3 p1CSO = mMesh.getVertex(faceIndices[0]).location;
		const glm::vec3 p2CSO = mMesh.getVertex(faceIndices[1]).location;
		const glm::vec3 p3CSO = mMesh.getVertex(faceIndices[2]).location;
		glm::vec3 closestPoint = getClosestPointInPlane(glm::vec3(0.0f), { p1CSO, p2CSO, p3CSO });
		float distance = glm::length(closestPoint);
		glm::vec3 closestPointBarycentricCoords;
		bool inside = projectPointOnTriangle(
			closestPoint, { p1CSO, p2CSO, p3CSO },
			mPrecision, closestPointBarycentricCoords
		);
		mFaceDistances[iFace] = { closestPoint, distance, inside, closestPointBarycentricCoords };

		return iFace;
	}


	void Polytope::removeFace(int iFace)
	{
		mMesh.removeFace(iFace);
		mFaceNormals.erase(iFace);
		mFaceDistances.erase(iFace);
	}

// Private functions
	void Polytope::tetrahedronFromEdge(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const std::vector<SupportPoint>& simplex
	) {
		glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();

		// 1. Find the coordinate axis that is the closest to be orthonormal
		// to the direction v01
		glm::vec3 vAxis(0.0f);
		auto absCompare = [](float f1, float f2) { return std::abs(f1) < std::abs(f2); };
		int iAxis = std::distance(&v01.x, std::min_element(&v01.x, &v01.x + 3, absCompare));
		vAxis[iAxis] = 1.0f;

		// 2. Calculate an orthonormal vector to the vector v01 with the axis
		glm::vec3 vNormal = glm::cross(v01, vAxis);

		// 3. Calculate 3 new points around the vector v01 by rotating
		// vNormal by 2*pi/3 radians around v01
		glm::mat3 rotate2Pi3 = glm::mat3(glm::rotate(glm::mat4(1.0f), 2.0f * glm::pi<float>() / 3.0f, v01));
		glm::vec3 searchDir = vNormal;
		for (int i = 0; i < 3; ++i) {
			addVertex(SupportPoint(collider1, collider2, searchDir));
			searchDir = rotate2Pi3 * searchDir;
		}

		// 4. The fourth point of the polytope must be either simplex[0] or
		// simplex[1], we select the one that creates a tetrahedron with the
		// origin inside simplex[3]
		glm::vec3 a = mVertexSupportPoints.find(0)->second.getCSOPosition();
		glm::vec3 b = mVertexSupportPoints.find(1)->second.getCSOPosition();
		glm::vec3 c = mVertexSupportPoints.find(2)->second.getCSOPosition();
		glm::vec3 tNormal = glm::cross(b - a, c - a);
		if (glm::dot(-a, tNormal) < 0) {
			tNormal = -tNormal;
		}

		if (glm::dot(simplex[0].getCSOPosition() - a, tNormal) > sKEpsilon) {
			addVertex(simplex[0]);
		}
		else {
			addVertex(simplex[1]);
		}

		// 5. Create the faces
		createTetrahedronFaces();
	}


	void Polytope::tetrahedronFromTriangle(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const std::vector<SupportPoint>& simplex
	) {
		// Search a support point along the simplex's triangle normal
		glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();
		glm::vec3 v02 = simplex[2].getCSOPosition() - simplex[0].getCSOPosition();
		glm::vec3 tNormal = glm::cross(v01, v02);

		SupportPoint sp(collider1, collider2, tNormal);

		if (glm::dot(tNormal, sp.getCSOPosition() - simplex[0].getCSOPosition()) < sKEpsilon) {
			// Try the opposite direction
			sp = SupportPoint(collider1, collider2, -tNormal);
		}

		addVertex(simplex[0]);
		addVertex(simplex[1]);
		addVertex(simplex[2]);
		addVertex(sp);
		createTetrahedronFaces();
	}


	void Polytope::createTetrahedronFaces()
	{
		addFace({ 0, 1, 2 });
		addFace({ 0, 3, 1 });
		addFace({ 0, 2, 3 });
		addFace({ 1, 3, 2 });
	}

}}
