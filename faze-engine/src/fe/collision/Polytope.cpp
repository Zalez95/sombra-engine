#include <exception>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Polytope.h"

namespace fe { namespace collision {

	PolytopeFace::PolytopeFace(
		const std::array<int, 3>& indices,
		const std::vector<SupportPoint>& vertices,
		float precision
	) {
		int iP1 = indices[0], iP2 = indices[1], iP3 = indices[2];
		glm::vec3 p1CSO = vertices[iP1].getCSOPosition();
		glm::vec3 p2CSO = vertices[iP2].getCSOPosition();
		glm::vec3 p3CSO = vertices[iP3].getCSOPosition();

		glm::vec3 normal = glm::normalize(glm::cross(p2CSO - p1CSO, p3CSO - p1CSO));
		if (glm::dot(normal, -p1CSO) > 0.0f) {
			// Fix the tetrahedron winding (the face must be pointing away from
			// the origin)
			normal = -normal;
			std::swap(iP2, iP3);
			std::swap(p2CSO, p3CSO);
		}

		triangle = { { iP1, iP2 }, { iP2, iP3 }, { iP3, iP1 }, normal };
		obsolete = false;
		closestPoint = getClosestPointInPlane(glm::vec3(0.0f), { p1CSO, p2CSO, p3CSO });
		distance = glm::length(closestPoint);
		inside = projectPointOnTriangle(
			closestPoint, { p1CSO, p2CSO, p3CSO },
			precision, closestPointBarycentricCoords
		);
	}


	Polytope::Polytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const std::vector<SupportPoint>& simplex, float precision
	) {
		switch(simplex.size()) {
			case 0:
			case 1:
				return;		// Don't calculate the initial tetrahedron polytope
			case 2:
				tetrahedronFromEdge(collider1, collider2, simplex, precision);
				break;
			case 3:
				tetrahedronFromTriangle(collider1, collider2, simplex, precision);
				break;
			default:
				vertices = { simplex[0], simplex[1], simplex[2], simplex[3] };
				createTetrahedronFaces(precision);
				break;
		}
	}


	void Polytope::addFace(const PolytopeFace& polytopeFace)
	{
		faces.insert(
			std::upper_bound(
				faces.begin(), faces.end(), polytopeFace,
				[](const PolytopeFace& p1, const PolytopeFace& p2) {
					return (p1.distance < p2.distance);
				}
			),
			polytopeFace
		);
	}

// Private functions
	void Polytope::tetrahedronFromEdge(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const std::vector<SupportPoint>& simplex, float precision
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
			vertices.emplace_back(collider1, collider2, searchDir);
			searchDir = rotate2Pi3 * searchDir;
		}

		// 4. The fourth point of the polytope must be either simplex[0] or
		// simplex[1], we select the one that creates a tetrahedron with the
		// origin inside
		glm::vec3 a = vertices[0].getCSOPosition(), b = vertices[1].getCSOPosition(), c = vertices[2].getCSOPosition();
		glm::vec3 tNormal = glm::cross(b - a, c - a);
		if (glm::dot(-a, tNormal) < 0) {
			tNormal = -tNormal;
		}

		if (glm::dot(simplex[0].getCSOPosition() - a, tNormal) > sKEpsilon) {
			vertices.push_back(simplex[0]);
		}
		else {
			vertices.push_back(simplex[1]);
		}

		// 5. Create the faces
		createTetrahedronFaces(precision);
	}


	void Polytope::tetrahedronFromTriangle(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		const std::vector<SupportPoint>& simplex, float precision
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

		vertices = { simplex[0], simplex[1], simplex[2], sp };
		createTetrahedronFaces(precision);
	}


	void Polytope::createTetrahedronFaces(float precision)
	{
		addFace( PolytopeFace({ 0, 1, 2 }, vertices, precision) );
		addFace( PolytopeFace({ 0, 3, 1 }, vertices, precision) );
		addFace( PolytopeFace({ 0, 2, 3 }, vertices, precision) );
		addFace( PolytopeFace({ 1, 3, 2 }, vertices, precision) );
	}

}}
