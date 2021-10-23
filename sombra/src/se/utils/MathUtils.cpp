#include <algorithm>
#include "se/utils/MathUtils.h"

namespace se::utils {

	glm::vec3 getClosestPointInEdge(
		const glm::vec3& p,
		const glm::vec3& e1, const glm::vec3& e2
	) {
		glm::vec3 ve1p = p - e1, ve2p = p - e2, ve1e2 = e2 - e1;
		float distance = glm::length(ve1e2);
		if (distance > 0.0f) {
			ve1e2 /= distance;
		}

		float lambda = glm::dot(ve1p, ve1e2);
		if (lambda < 0.0f) {
			lambda = 0.0f;
		}
		else if (glm::dot(ve2p, ve1e2) > 0.0f) {
			lambda = distance;
		}

		return e1 + lambda * ve1e2;
	}


	std::pair<bool, glm::vec2> projectPointOnEdge(
		const glm::vec3& point, const std::array<glm::vec3, 2>& edge,
		float projectionPrecision
	) {
		glm::vec3 v0v1 = edge[1] - edge[0], v0p = point - edge[0];

		float v0v1Length = glm::length(v0v1);
		glm::vec3 v0v1norm = (v0v1Length > 0.0f)? v0v1 / v0v1Length : glm::vec3(0.0f);
		float beta	= glm::length(glm::dot(v0p, v0v1norm) * v0v1norm) / v0v1Length;
		float alpha	= 1.0f - beta;

		bool inside = ((-projectionPrecision <= alpha) && (alpha <= 1.0f + projectionPrecision)
					&& (-projectionPrecision <= beta) && (beta <= 1.0f + projectionPrecision));
		glm::vec2 projectedPoint = glm::vec2(alpha, beta);
		return std::make_pair(inside, projectedPoint);
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


	float signedDistancePlanePoint(const glm::vec4& plane, const glm::vec3& point)
	{
		return glm::dot(glm::vec3(plane), point) + plane.w;
	}


	std::pair<bool, glm::vec3> projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		float projectionPrecision
	) {
		glm::vec3 u = triangle[1] - triangle[0], v = triangle[2] - triangle[0], w = point - triangle[0];
		glm::vec3 n = glm::cross(u, v);

		float nLengthSq = glm::dot(n, n);
		float gamma	= glm::dot(glm::cross(u, w), n) / nLengthSq;
		float beta	= glm::dot(glm::cross(w, v), n) / nLengthSq;
		float alpha	= 1.0f - gamma - beta;

		bool inside = ((-projectionPrecision <= alpha) && (alpha <= 1.0f + projectionPrecision)
					&& (-projectionPrecision <= beta) && (beta <= 1.0f + projectionPrecision)
					&& (-projectionPrecision <= gamma) && (gamma <= 1.0f + projectionPrecision));
		glm::vec3 projectedPoint = glm::vec3(alpha, beta, gamma);
		return std::make_pair(inside, projectedPoint);
	}


	float calculateTriangleArea(const std::array<glm::vec3, 3>& triangle)
	{
		glm::vec3 v12 = triangle[1] - triangle[0];
		glm::vec3 v13 = triangle[2] - triangle[0];
		return 0.5f * glm::length(glm::cross(v12, v13));
	}


	bool compareTriangles(
		const std::array<glm::vec3, 3>& triangle1,
		const std::array<glm::vec3, 3>& triangle2,
		float epsilon
	) {
		bool sameVertices = std::all_of(triangle1.begin(), triangle1.end(), [&](const glm::vec3& v1) {
			return std::any_of(triangle2.begin(), triangle2.end(), [&](const glm::vec3& v2) {
				return glm::all(glm::epsilonEqual(v1, v2, epsilon));
			});
		});

		glm::vec3 normal1 = glm::cross(triangle1[1] - triangle1[0], triangle1[2] - triangle1[0]);
		glm::vec3 normal2 = glm::cross(triangle2[1] - triangle2[0], triangle2[2] - triangle2[0]);
		bool sameNormalDirection = glm::dot(normal1, normal2) > 0;

		return sameVertices && sameNormalDirection;
	}


	std::pair<glm::vec3, glm::vec3> getBoundsWorld(
		const glm::vec3& localMin, const glm::vec3& localMax,
		const glm::mat4& worldMatrix
	) {
		std::array<glm::vec3, 8> localAABBVertices = {
			glm::vec3(localMin.x, localMin.y, localMin.z),
			glm::vec3(localMin.x, localMin.y, localMax.z),
			glm::vec3(localMin.x, localMax.y, localMin.z),
			glm::vec3(localMin.x, localMax.y, localMax.z),
			glm::vec3(localMax.x, localMin.y, localMin.z),
			glm::vec3(localMax.x, localMin.y, localMax.z),
			glm::vec3(localMax.x, localMax.y, localMin.z),
			glm::vec3(localMax.x, localMax.y, localMax.z)
		};

		glm::vec3 worldMin, worldMax;
		worldMin = worldMax = worldMatrix * glm::vec4(localAABBVertices[0], 1.0f);
		for (std::size_t i = 1; i < 8; ++i) {
			glm::vec3 vw = worldMatrix * glm::vec4(localAABBVertices[i], 1.0f);
			worldMin = glm::min(worldMin, vw);
			worldMax = glm::max(worldMax, vw);
		}

		return { worldMin, worldMax };
	}


	void decompose(
		const glm::mat4& transforms,
		glm::vec3& translation, glm::quat& rotation, glm::vec3& scale
	) {
		translation = transforms[3];
		scale = glm::vec3(glm::length(transforms[0]), glm::length(transforms[1]), glm::length(transforms[2]));
		rotation = glm::quat_cast( glm::mat3(transforms[0] / scale.x, transforms[1] / scale.y, transforms[2] / scale.z) );
	}

}
