#include <algorithm>
#include <glm/gtc/epsilon.hpp>
#include "se/collision/Simplex.h"

namespace se::collision {

	bool isOriginInside(const Simplex& simplex, float epsilon)
	{
		switch (simplex.size()) {
			case 1: {
				if (!glm::all(glm::epsilonEqual(simplex[0].getCSOPosition(), glm::vec3(0.0f), epsilon))) {
					return false;
				}
			} break;
			case 2: {
				glm::vec3 v1v2 = glm::normalize(simplex[1].getCSOPosition() - simplex[0].getCSOPosition());
				float dot1 = glm::dot(v1v2, -simplex[0].getCSOPosition());
				float dot2 = glm::dot(v1v2, -simplex[1].getCSOPosition());
				if ((dot1 < -epsilon) || (dot2 > -epsilon)) {
					return false;
				}
				else {
					glm::vec3 proj = simplex[0].getCSOPosition() + dot1 * v1v2;
					if (!glm::all(glm::epsilonEqual(proj, glm::vec3(0.0f), epsilon))) {
						return false;
					}
				}
			} break;
			case 3: {
				glm::vec3 v1v2 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();
				glm::vec3 v1v3 = simplex[2].getCSOPosition() - simplex[0].getCSOPosition();
				glm::vec3 tNormal = glm::normalize(glm::cross(v1v2, v1v3));
				if (glm::dot(tNormal, simplex[0].getCSOPosition()) > -epsilon) {
					return false;
				}
			} break;
			case 4: {
				for (int i = 0; i < 4; ++i) {
					int iV1 = i % 3, iV2 = (i + 1) % 3, iV3 = (i + 2) % 3;

					glm::vec3 v1v2 = simplex[iV2].getCSOPosition() - simplex[iV1].getCSOPosition();
					glm::vec3 v1v3 = simplex[iV3].getCSOPosition() - simplex[iV1].getCSOPosition();
					glm::vec3 tNormal = glm::normalize(glm::cross(v1v2, v1v3));
					if (glm::dot(tNormal, simplex[iV1].getCSOPosition()) > -epsilon) {
						return false;
					}
				}
			} break;
			default:
				return false;
		}

		return true;
	}


	bool isClose(const Simplex& simplex, const glm::vec3& point, float epsilon)
	{
		return std::any_of(simplex.begin(), simplex.end(), [&](const SupportPoint& sp) {
			return glm::all(glm::epsilonEqual(sp.getCSOPosition(), point, epsilon));
		});
	}

}
