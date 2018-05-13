#include "fe/collision/Edge.h"

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

}}
