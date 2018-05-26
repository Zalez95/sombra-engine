#include "fe/collision/SupportPoint.h"
#include "Triangle.h"

namespace fe { namespace collision {

	Triangle::Triangle(SupportPoint* a, SupportPoint* b, SupportPoint* c) :
		ab{a, b}, bc{b, c}, ca{c, a}
	{
		normal = glm::normalize(glm::cross(
			bc.p1->getCSOPosition() - ab.p1->getCSOPosition(),
			ca.p1->getCSOPosition() - ab.p1->getCSOPosition()
		));
	}


	bool Triangle::operator==(const Triangle& other) const
	{
		if (ab == other.ab) {
			return (bc == other.bc) && (ca == other.ca);
		}
		else if (ab == other.bc) {
			return (bc == other.ca) && (ca == other.ab);
		}
		else if (ab == other.ca) {
			return (bc == other.ab) && (ca == other.bc);
		}
		else {
			return false;
		}
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

}}
