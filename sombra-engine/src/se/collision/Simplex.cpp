#include <algorithm>
#include <glm/gtc/epsilon.hpp>
#include "se/utils/MathUtils.h"
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


	std::pair<bool, SupportPoint> calculateClosestPoint(const Simplex& simplex, float epsilon)
	{
		bool success = false;
		SupportPoint value;

		switch (simplex.size()) {
			case 1: {
				value = simplex[0];
				success = true;
			} break;
			case 2: {
				auto [inside, originBarycentricCoords] = utils::projectPointOnEdge(
					glm::vec3(0.0f),
					{ simplex[0].getCSOPosition(), simplex[1].getCSOPosition() },
					epsilon
				);
				if (!inside) {
					originBarycentricCoords = glm::clamp(originBarycentricCoords, glm::vec2(0.0f), glm::vec2(1.0f));
				}

				glm::vec3 worldPosition[2], localPosition[2];
				for (int i = 0; i < 2; ++i) {
					worldPosition[i] = originBarycentricCoords.x * simplex[0].getWorldPosition(i)
						+ originBarycentricCoords.y * simplex[1].getWorldPosition(i);
					localPosition[i] = originBarycentricCoords.x * simplex[0].getLocalPosition(i)
						+ originBarycentricCoords.y * simplex[1].getLocalPosition(i);
				}
				value = SupportPoint(worldPosition[0], localPosition[0], worldPosition[1], localPosition[1]);
				success = true;
			} break;
			case 3: {
				auto [inside, originBarycentricCoords] = utils::projectPointOnTriangle(
					glm::vec3(0.0f),
					{ simplex[0].getCSOPosition(), simplex[1].getCSOPosition(), simplex[2].getCSOPosition() },
					epsilon
				);
				if (!inside) {
					originBarycentricCoords = glm::clamp(originBarycentricCoords, glm::vec3(0.0f), glm::vec3(1.0f));
				}

				glm::vec3 worldPosition[2], localPosition[2];
				for (int i = 0; i < 2; ++i) {
					worldPosition[i] = originBarycentricCoords.x * simplex[0].getWorldPosition(i)
						+ originBarycentricCoords.y * simplex[1].getWorldPosition(i)
						+ originBarycentricCoords.z * simplex[2].getWorldPosition(i);
					localPosition[i] = originBarycentricCoords.x * simplex[0].getLocalPosition(i)
						+ originBarycentricCoords.y * simplex[1].getLocalPosition(i)
						+ originBarycentricCoords.z * simplex[2].getLocalPosition(i);
				}
				value = SupportPoint(worldPosition[0], localPosition[0], worldPosition[1], localPosition[1]);
				success = true;
			} break;
			case 4: {
				float minDistance = std::numeric_limits<float>::max();
				for (int i = 0; i < 4; ++i) {
					int iV1 = i % 3, iV2 = (i + 1) % 3, iV3 = (i + 2) % 3;

					auto [inside, originBarycentricCoords] = utils::projectPointOnTriangle(
						glm::vec3(0.0f),
						{ simplex[iV1].getCSOPosition(), simplex[iV2].getCSOPosition(), simplex[iV3].getCSOPosition() },
						epsilon
					);
					if (!inside) {
						originBarycentricCoords = glm::clamp(originBarycentricCoords, glm::vec3(0.0f), glm::vec3(1.0f));
					}

					glm::vec3 worldPosition[2], localPosition[2];
					for (int j = 0; j < 2; ++j) {
						worldPosition[j] = originBarycentricCoords.x * simplex[iV1].getWorldPosition(j)
							+ originBarycentricCoords.y * simplex[iV2].getWorldPosition(j)
							+ originBarycentricCoords.z * simplex[iV3].getWorldPosition(j);
						localPosition[j] = originBarycentricCoords.x * simplex[iV1].getLocalPosition(j)
							+ originBarycentricCoords.y * simplex[iV2].getLocalPosition(j)
							+ originBarycentricCoords.z * simplex[iV3].getLocalPosition(j);
					}
					SupportPoint sp(worldPosition[0], localPosition[0], worldPosition[1], localPosition[1]);

					float currentDistance = glm::dot(sp.getCSOPosition(), sp.getCSOPosition());
					if (currentDistance < minDistance) {
						value = sp;
						success = true;
					}
				}
			} break;
			default:
				success = false;
				break;
		}

		return std::make_pair(success, value);
	}

}
