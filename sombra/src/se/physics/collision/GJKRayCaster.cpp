#include <glm/gtc/random.hpp>
#include <glm/gtc/epsilon.hpp>
#include "se/physics/collision/ConvexCollider.h"
#include "se/physics/collision/CollisionMath.h"
#include "GJKRayCaster.h"

namespace se::physics {

	std::pair<bool, RayHit> GJKRayCaster::calculateRayCast(const Ray& ray, const ConvexCollider& collider) const
	{
		glm::vec3 pointRandomW(0.0f), pointRandomL(0.0f);
		collider.getFurthestPointInDirection(glm::sphericalRand(1.0f), pointRandomW, pointRandomL);

		float lambda = 0.0f;
		glm::vec3 x = ray.origin;
		glm::vec3 n(0.0f);
		glm::vec3 v = SupportPoint(x, glm::vec3(0.0f), pointRandomW, pointRandomL).getCSOPosition();
		Simplex simplex;
		utils::FixedVector<bool, 4> closestSimplexPoints;

		float dist2 = glm::dot(v, v);
		std::size_t iteration = 0;
		while ((dist2 > mEpsilon * mEpsilon) && (iteration < mMaxIterations)) {
			// Search a new point in the v direction
			glm::vec3 pointWorld(0.0f), pointLocal(0.0f);
			collider.getFurthestPointInDirection(v, pointWorld, pointLocal);

			glm::vec3 w = x - pointWorld;
			if (glm::dot(v, w) > mEpsilon) {
				if (glm::dot(v, ray.direction) >= -mEpsilon) {
					return { false, RayHit() };
				}
				else {
					lambda -= glm::dot(v, w) / glm::dot(v, ray.direction);
					x = ray.origin + lambda * ray.direction;
					n = v;
				}
			}

			// Update the world x position in the simplex
			Simplex newSimplex;
			for (const SupportPoint& sp : simplex) {
				newSimplex.emplace_back(x, glm::vec3(0.0f), sp.getWorldPosition(1), sp.getLocalPosition(1));
			}
			simplex = newSimplex;

			// Insert a new support point if it isn't close to the other points
			SupportPoint sp(x, glm::vec3(0.0f), pointWorld, pointLocal);
			if (!isClose(simplex, sp.getCSOPosition(), mEpsilon)) {
				simplex.push_back(sp);
				closestSimplexPoints.push_back(false);
			}

			// Calculate the closest point to the origin
			auto [success, closestPoint] = calculateClosestPoint(simplex, closestSimplexPoints);
			if (success) {
				v = closestPoint.getCSOPosition();
				dist2 = glm::dot(v, v);

				// Remove old simplex points
				reduce(simplex, closestSimplexPoints);
			}
			else {
				dist2 = 0.0f;
			}

			++iteration;
		}

		auto [success, closestPoint] = calculateClosestPoint(simplex, closestSimplexPoints);
		if (success) {
			float nLength = glm::length(n);

			RayHit rayHit;
			rayHit.distance = lambda;
			rayHit.contactPointLocal = closestPoint.getLocalPosition(1);
			rayHit.contactPointWorld = closestPoint.getWorldPosition(1);
			rayHit.contactNormal = (nLength > mEpsilon)? n / nLength : glm::vec3(0.0f);

			return { true, rayHit };
		}
		else {
			return { false, RayHit() };
		}
	}

// Private functions
	std::pair<bool, SupportPoint> GJKRayCaster::calculateClosestPoint(
		const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
	) const
	{
		switch (simplex.size()) {
			case 1:		return calculateClosestPoint1(simplex, closestPoints);	break;
			case 2:		return calculateClosestPoint2(simplex, closestPoints);	break;
			case 3:		return calculateClosestPoint3(simplex, closestPoints);	break;
			case 4:		return calculateClosestPoint4(simplex, closestPoints);	break;
			default:	return { false, SupportPoint() };						break;
		}
	}


	std::pair<bool, SupportPoint> GJKRayCaster::calculateClosestPoint1(
		const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
	) const
	{
		closestPoints[0] = true;
		return { true, simplex[0] };
	}


	std::pair<bool, SupportPoint> GJKRayCaster::calculateClosestPoint2(
		const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
	) const
	{
		auto [inside, originBarycentricCoords] = projectPointOnEdge(
			glm::vec3(0.0f),
			{ simplex[0].getCSOPosition(), simplex[1].getCSOPosition() },
			mEpsilon
		);
		if (!inside) {
			if (originBarycentricCoords[0] < -mEpsilon) {
				closestPoints[0] = true;
				closestPoints[1] = false;
			}
			else if (originBarycentricCoords[1] < -mEpsilon) {
				closestPoints[0] = false;
				closestPoints[1] = true;
			}

			originBarycentricCoords = glm::clamp(originBarycentricCoords, glm::vec2(0.0f), glm::vec2(1.0f));
		}
		else {
			closestPoints[0] = closestPoints[1] = true;
		}

		glm::vec3 worldPosition[2], localPosition[2];
		for (int i = 0; i < 2; ++i) {
			worldPosition[i] = originBarycentricCoords.x * simplex[0].getWorldPosition(i)
				+ originBarycentricCoords.y * simplex[1].getWorldPosition(i);
			localPosition[i] = originBarycentricCoords.x * simplex[0].getLocalPosition(i)
				+ originBarycentricCoords.y * simplex[1].getLocalPosition(i);
		}

		return { true, SupportPoint(worldPosition[0], localPosition[0], worldPosition[1], localPosition[1]) };
	}


	std::pair<bool, SupportPoint> GJKRayCaster::calculateClosestPoint3(
		const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
	) const
	{
		auto [inside, originBarycentricCoords] = projectPointOnTriangle(
			glm::vec3(0.0f),
			{ simplex[0].getCSOPosition(), simplex[1].getCSOPosition(), simplex[2].getCSOPosition() },
			mEpsilon
		);
		if (!inside) {
			closestPoints[0] = (originBarycentricCoords[0] >= -mEpsilon);
			closestPoints[1] = (originBarycentricCoords[1] >= -mEpsilon);
			closestPoints[2] = (originBarycentricCoords[2] >= -mEpsilon);

			originBarycentricCoords = glm::clamp(originBarycentricCoords, glm::vec3(0.0f), glm::vec3(1.0f));
		}
		else {
			closestPoints[0] = closestPoints[1] = closestPoints[2] = true;
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

		return { true, SupportPoint(worldPosition[0], localPosition[0], worldPosition[1], localPosition[1]) };
	}


	std::pair<bool, SupportPoint> GJKRayCaster::calculateClosestPoint4(
		const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
	) const
	{
		bool success = false;
		SupportPoint value;

		float minDistance = std::numeric_limits<float>::max();
		for (int i = 0; i < 4; ++i) {
			int iV1 = i % 3, iV2 = (i + 1) % 3, iV3 = (i + 2) % 3;
			utils::FixedVector<bool, 4> currentClosestPoints = { false, false, false, false };

			auto [inside, originBarycentricCoords] = projectPointOnTriangle(
				glm::vec3(0.0f),
				{ simplex[iV1].getCSOPosition(), simplex[iV2].getCSOPosition(), simplex[iV3].getCSOPosition() },
				mEpsilon
			);
			if (!inside) {
				currentClosestPoints[iV1] = (originBarycentricCoords[iV1] >= -mEpsilon);
				currentClosestPoints[iV2] = (originBarycentricCoords[iV2] >= -mEpsilon);
				currentClosestPoints[iV3] = (originBarycentricCoords[iV3] >= -mEpsilon);
				originBarycentricCoords = glm::clamp(originBarycentricCoords, glm::vec3(0.0f), glm::vec3(1.0f));
			}
			else {
				closestPoints[iV1] = closestPoints[iV2] = closestPoints[iV3] = true;
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
				closestPoints = currentClosestPoints;
			}
		}

		return { success, value };
	}


	void GJKRayCaster::reduce(Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints)
	{
		for (std::size_t i = 0; i < simplex.size();) {
			if (!closestPoints[i]) {
				simplex.erase(simplex.begin() + i);
				closestPoints.erase(closestPoints.begin() + i);
			}
			else {
				++i;
			}
		}
	}

}
