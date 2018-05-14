#include "fe/collision/SupportPoint.h"
#include "Triangle.h"

namespace fe { namespace collision {

	Triangle::Triangle(SupportPoint* a, SupportPoint* b, SupportPoint* c) :
		ab{a, b}, bc{b, c}, ca{c, a}
	{
		normal = glm::normalize(glm::cross(
			ab.p2->getCSOPosition() - ab.p1->getCSOPosition(),
			ca.p2->getCSOPosition() - ca.p1->getCSOPosition()
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


	float distancePointTriangle(
		const glm::vec3& p,
		const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3
	) {
		glm::vec3 vt1t2	= t1 - t2, vt2t3 = t2 - t3, vt3t1 = t3 - t1,
			tNormal = glm::normalize(-glm::cross(vt1t2, vt3t1));

		glm::vec3 vt1p = p - t1, vt1t2xtNormal = glm::normalize(glm::cross(vt1t2, tNormal));
		if (glm::dot(vt1p, vt1t2xtNormal) > 0) {
			return distancePointEdge(p, t1, t2);
		}

		glm::vec3 vt2p = p - t2, vt2t3xtNormal = glm::normalize(glm::cross(vt2t3, tNormal));
		if (glm::dot(vt2p, vt2t3xtNormal) > 0) {
			return distancePointEdge(p, t2, t3);
		}

		glm::vec3 vt3p = p - t3, vt3t1xtNormal = glm::normalize(glm::cross(vt3t1, tNormal));
		if (glm::dot(vt3p, vt3t1xtNormal) > 0) {
			return distancePointEdge(p, t3, t1);
		}

		return abs(glm::dot(vt1p, tNormal));
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

		if ((0.0f - projectionPrecision <= gamma) && (gamma <= 1.0f + projectionPrecision)
			&& (0.0f - projectionPrecision <= beta) && (beta <= 1.0f + projectionPrecision)
			&& (0.0f - projectionPrecision <= alpha) && (alpha <= 1.0f + projectionPrecision)
		) {
			projectedPoint = glm::vec3(gamma, beta, alpha);
			return true;
		}

		return false;
	}

}}
