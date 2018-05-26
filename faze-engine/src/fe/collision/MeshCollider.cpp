#include <limits>
#include <cassert>
#include "fe/collision/MeshCollider.h"

namespace fe { namespace collision {

	MeshCollider::MeshCollider(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices,
		ConvexStrategy strategy
	) : mTransformsMatrix(1.0f)
	{
		assert(indices.size() % 3 == 0 && "The faces of the MeshCollider must be triangles");

		//calculateAABBs();
		if (strategy == ConvexStrategy::QuickHull) {
			doQuickHull(vertices);
		}

		calculateAABB();
	}


	void MeshCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (ConvexPolyhedron& convexPart : mConvexParts) {
			convexPart.setTransforms(transforms);
		}

		calculateAABB();
	}


	std::vector<const ConvexCollider*> MeshCollider::getOverlapingParts(const AABB& aabb) const
	{
		std::vector<const ConvexCollider*> overlapingParts;

		for (const ConvexPolyhedron& convexPart : mConvexParts) {
			if (overlaps(aabb, convexPart.getAABB())) {
				overlapingParts.push_back(&convexPart);
			}
		}

		return overlapingParts;
	}

// Private functions
	void MeshCollider::calculateAABB()
	{
		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (const ConvexPolyhedron& convexPart : mConvexParts) {
			AABB convexPartAABB = convexPart.getAABB();
			mAABB.minimum = glm::min(mAABB.minimum, convexPartAABB.minimum);
			mAABB.maximum = glm::max(mAABB.maximum, convexPartAABB.maximum);
		}
	}


	std::vector<glm::vec3> MeshCollider::doQuickHull(
		const std::vector<glm::vec3>& points
	) const
	{
		std::vector<glm::vec3> convexHull = createInitialHull(points);


		return convexHull;
	}


	std::vector<glm::vec3> MeshCollider::createInitialHull(
		const std::vector<glm::vec3>& points
	) const
	{
		size_t iP1, iP2, iP3, iP4;
		
		// 1. Find the extreme points in each axis
		std::array<size_t, 6> indexExtremePoints{};
		for (size_t i = 0; i < points.size(); ++i) {
			for (size_t j = 0; j < 3; ++j) {
				if (points[i][j] < points[indexExtremePoints[2*j]][j]) {
					indexExtremePoints[2*j] = i;
				}
				if (points[i][j] > points[indexExtremePoints[2*j + 1]][j]) {
					indexExtremePoints[2*j + 1] = i;
				}
			}
		}

		// 2. Find from the extreme points the pair which are furthest apart
		float maxLength = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size(); ++i) {
			for (size_t j = i + 1; j < points.size(); ++j) {
				float currentLength = glm::length(points[j] - points[i]);
				if (currentLength > maxLength) {
					iP1 = i; iP2 = j;
					maxLength = currentLength;
				}
			}
		}

		// 3. Find the furthest point to the edge between the last 2 points
		glm::vec3 dirP1P2 = glm::normalize(points[iP2] - points[iP1]);
		maxLength = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size(); ++i) {
			glm::vec3 projection = points[iP1] + dirP1P2 * glm::dot(points[i], dirP1P2);
			float currentLength = glm::length(points[i] - projection);
			if (currentLength > maxLength) {
				iP3 = i;
				maxLength = currentLength;
			}
		}

		// 4. Find the furthest point to the triangle created from the last 3
		// points
		glm::vec3 dirP1P3 = glm::normalize(points[iP3] - points[iP1]);
		glm::vec3 tNormal = glm::normalize(glm::cross(dirP1P2, dirP1P3));
		maxLength = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < points.size(); ++i) {
			float currentLength = std::abs(glm::dot(points[i], tNormal));
			if (currentLength > maxLength) {
				iP4 = i;
				maxLength = currentLength;
			}
		}

		return { points[iP1], points[iP2], points[iP3], points[iP4] };
	}

}}
