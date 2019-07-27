#include "se/utils/MathUtils.h"

namespace se::utils {

	glm::vec3 getClosestPointInEdge(
		const glm::vec3& p,
		const glm::vec3& e1, const glm::vec3& e2
	) {
		glm::vec3 ret(0.0f);

		glm::vec3 ve1p = p - e1, ve2p = p - e2, ve1e2 = glm::normalize(e2 - e1);
		if (float dot1 = glm::dot(ve1p, ve1e2) <= 0.0f) {
			ret = e1;
		}
		else if (glm::dot(ve2p, ve1e2) >= 0.0f) {
			ret = e2;
		}
		else {
			ret = e1 + dot1 * ve1e2;
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


	std::pair<bool, glm::vec3> rayPlaneIntersection(
		const glm::vec3& point, const glm::vec3& direction,
		const glm::vec3& planePoint, const glm::vec3& planeNormal,
		float intersectionPrecision
	) {
		bool intersects = false;
		glm::vec3 intersection, pointToPlanePoint = planePoint - point;

		float dotPPN = glm::dot(pointToPlanePoint, planeNormal);
		if (dotPPN > intersectionPrecision) {
			float dotDN = glm::dot(direction, planeNormal);
			if (dotDN > intersectionPrecision) {
				intersection = point + direction * (dotPPN / dotDN);
				intersects = true;
			}
		}
		else if (dotPPN < -intersectionPrecision) {
			float dotDN = glm::dot(direction, planeNormal);
			if (dotDN < -intersectionPrecision) {
				intersection = point + direction * (dotPPN / dotDN);
				intersects = true;
			}
		}
		else {
			// The origin is on the HEFace
			intersection = point;
			intersects = true;
		}

		return std::make_pair(intersects, intersection);
	}


	bool projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		float projectionPrecision, glm::vec3& projectedPoint
	) {
		glm::vec3 u = triangle[1] - triangle[0], v = triangle[2] - triangle[0], w = point - triangle[0];
		glm::vec3 n = glm::cross(u, v);

		float nLengthSq = glm::dot(n, n);
		float gamma	= glm::dot(glm::cross(u, w), n) / nLengthSq;
		float beta	= glm::dot(glm::cross(w, v), n) / nLengthSq;
		float alpha	= 1.0f - gamma - beta;

		if ((-projectionPrecision <= alpha) && (alpha <= 1.0f + projectionPrecision)
			&& (-projectionPrecision <= beta) && (beta <= 1.0f + projectionPrecision)
			&& (-projectionPrecision <= gamma) && (gamma <= 1.0f + projectionPrecision)
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
		return 0.5f * glm::length(glm::cross(v12, v13));
	}

}
