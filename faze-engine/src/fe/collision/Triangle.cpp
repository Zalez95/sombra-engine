#include "fe/collision/SupportPoint.h"
#include "Triangle.h"

namespace fe { namespace collision {


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
		glm::vec3 u = triangle[1] - triangle[0], v = triangle[2] - triangle[0],
			w = point - triangle[0], n = glm::cross(u, v);

		float gamma	= glm::dot(glm::cross(u, w), n) / glm::dot(n, n);
		float beta	= glm::dot(glm::cross(w, v), n) / glm::dot(n, n);
		float alpha	= 1 - gamma - beta;

		if ((0.0f - projectionPrecision <= alpha) && (alpha <= 1.0f + projectionPrecision)
			&& (0.0f - projectionPrecision <= beta) && (beta <= 1.0f + projectionPrecision)
			&& (0.0f - projectionPrecision <= gamma) && (gamma <= 1.0f + projectionPrecision)
		) {
			projectedPoint = glm::vec3(alpha, beta, gamma);
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

}}