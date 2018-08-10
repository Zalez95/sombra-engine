#include "Geometry.h"

namespace fe { namespace collision {

	float distancePointEdge(
		const glm::vec3& p,
		const glm::vec3& e1, const glm::vec3& e2
	) {
		float ret = -1;

		glm::vec3 ve1p = p - e1, ve2p = p - e2, ve1e2 = glm::normalize(e2 - e1);
		if (float dot1 = glm::dot(ve1p, ve1e2) < 0) {
			ret = glm::length(ve1p);
		}
		else if (glm::dot(ve2p, ve1e2) > 0) {
			ret = glm::length(ve2p);
		}
		else {
			ret = glm::length(p - (e1 + dot1 * ve1e2));
		}

		return ret;
	}


	glm::vec3 getClosestPointInPlane(
		const glm::vec3& p,
		const std::array<glm::vec3, 3>& planePoints
	) {
		glm::vec3 vp1p = p - planePoints[0],
			vp1p2 = planePoints[1] - planePoints[0], vp1p3 = planePoints[2] - planePoints[0],
			pNormal = glm::normalize(glm::cross(vp1p2, vp1p3));

		float distance = glm::dot(vp1p, pNormal);
		return p - distance * pNormal;
	}


	bool projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		float projectionPrecision, glm::vec3& projectedPoint
	) {
		glm::vec3 v0 = triangle[1] - triangle[0], v1 = triangle[2] - triangle[0], v2 = point - triangle[0];
		float den = 1.0f / (v0.x * v1.y - v1.x * v0.y);

		float v = (v2.x * v1.y - v1.x * v2.y) * den;
		float w = (v0.x * v2.y - v2.x * v0.y) * den;
		float u = 1.0f - v - w;

		if ((0.0f - projectionPrecision <= u) && (u <= 1.0f + projectionPrecision)
			&& (0.0f - projectionPrecision <= v) && (v <= 1.0f + projectionPrecision)
			&& (0.0f - projectionPrecision <= w) && (w <= 1.0f + projectionPrecision)
		) {
			projectedPoint = glm::vec3(u, v, w);
			return true;
		}

		return false;
	}


	float calculateTriangleArea(const std::array<glm::vec3, 3>& triangle)
	{
		glm::vec3 v12 = triangle[1] - triangle[0];
		glm::vec3 v13 = triangle[2] - triangle[0];
		float sinTheta = std::sqrt(1.0f - std::pow(glm::dot(v12, v13), 2.0f));
		return glm::length(v12) * glm::length(v13) * sinTheta / 2.0f;
	}


	float calculatePolygonPerimeter(const std::vector<glm::vec3>& vertices)
	{
		float perimeter = 0.0f;
		for (size_t i = 0; i < vertices.size(); ++i) {
			perimeter += glm::length(vertices[(i + 1) % vertices.size()] - vertices[i]);
		}
		return perimeter;
	}

}}
