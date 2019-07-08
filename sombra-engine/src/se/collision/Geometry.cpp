#include "Geometry.h"

namespace se::collision {

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
		return 0.5f * std::sqrt(
			std::pow(v12.y * v13.z - v12.z * v13.y, 2)
			+ std::pow(v12.z * v13.x - v12.x * v13.z, 2)
			+ std::pow(v12.x * v13.y - v12.y * v13.x, 2)
		);
	}

}
