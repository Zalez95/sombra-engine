#include <glm/gtc/random.hpp>
#include <glm/gtc/epsilon.hpp>
#include "se/collision/ConvexCollider.h"
#include "se/collision/GJKRayCaster.h"
#include "se/collision/Simplex.h"

namespace se::collision {

	std::pair<bool, RayCast> GJKRayCaster::calculateRayCast(
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		const ConvexCollider& collider
	) const
	{
		glm::vec3 pointRandomW(0.0f), pointRandomL(0.0f);
		collider.getFurthestPointInDirection(glm::sphericalRand(1.0f), pointRandomW, pointRandomL);

		float lambda = 0.0f;
		glm::vec3 x = rayOrigin;
		glm::vec3 n(0.0f);
		SupportPoint v(x, glm::vec3(0.0f), pointRandomW, pointRandomL);
		utils::FixedVector<std::pair<glm::vec3, glm::vec3>, 4> points;

		float dist2 = glm::dot(v.getCSOPosition(), v.getCSOPosition());
		while (dist2 > mEpsilon * mEpsilon) {
			glm::vec3 pointWorld(0.0f), pointLocal(0.0f);
			collider.getFurthestPointInDirection(v.getCSOPosition(), pointWorld, pointLocal);

			glm::vec3 w = x - pointWorld;
			if (glm::dot(v.getCSOPosition(), w) > mEpsilon) {
				if (glm::dot(v.getCSOPosition(), rayDirection) >= -mEpsilon) {
					return { false, RayCast{} };
				}
				else {
					lambda -= glm::dot(v.getCSOPosition(), w) / glm::dot(v.getCSOPosition(), rayDirection);
					x = rayOrigin + lambda * rayDirection;
					n = v.getCSOPosition();
				}
			}

			if (!points.full()) {
				points.emplace_back(pointWorld, pointLocal);

				Simplex simplex;
				for (const auto& [pw, pl] : points) {
					SupportPoint sp(x, glm::vec3(0.0f), pw, pl);
					if (!isClose(simplex, sp.getCSOPosition(), mEpsilon)) {
						simplex.push_back(sp);
					}
				}

				auto [success, closestPoint] = calculateClosestPoint(simplex, mEpsilon);
				if (success) {
					v = closestPoint;
					dist2 = glm::dot(v.getCSOPosition(), v.getCSOPosition());
				}
			}
			else {
				dist2 = 0.0f;
			}
		}

		RayCast rayCast;
		rayCast.distance = lambda;
		rayCast.contactPointLocal = v.getLocalPosition(1);
		rayCast.contactPointWorld = v.getWorldPosition(1);
		rayCast.contactNormal = v.getCSOPosition();

		return { true, rayCast };
	}

}
