#include <limits>
#include <cassert>
#include <algorithm>
#include "fe/collision/TerrainCollider.h"
#include "fe/collision/ConvexPolyhedron.h"

namespace fe { namespace collision {

	TerrainCollider::TerrainCollider(
		const std::vector<float>& heights, int xSize, int zSize
	) : mHeights(heights), mXSize(xSize), mZSize(zSize),
		mTransformsMatrix(1.0f), mInverseTransformsMatrix(1.0f)
	{
		assert(static_cast<int>(mHeights.size()) >= mXSize * mZSize
			&& "There aren't enough heights");
		calculateAABB();
	}


	void TerrainCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mInverseTransformsMatrix = glm::inverse(mTransformsMatrix);
		calculateAABB();
	}


	std::vector<const ConvexCollider*> TerrainCollider::getOverlapingParts(const AABB& /*aabb*/) const
	{
		std::vector<const ConvexCollider*> triangleColliders;
/*
		auto xIndexToX = [this](int i) { return i / static_cast<float>(mXSize - 1) - 0.5f; };
		auto zIndexToZ = [this](int i) { return i / static_cast<float>(mZSize - 1) - 0.5f; };

		// Get the AABB in local coordinates
		AABB localAABB = calculateLocalAABB(aabb);

		// Calculate the indices of the terrain vertices to check with the
		// local position of the AABB
		int iMinX = mXSize * (localAABB.minimum.x + 0.5f), iMaxX = std::ceil(mXSize * (localAABB.maximum.x + 0.5f)),
			iMinZ = mZSize * (localAABB.minimum.z + 0.5f), iMaxZ = std::ceil(mZSize * (localAABB.maximum.z + 0.5f));
		if ((iMinX < mXSize) && (iMaxX >= 0) && (iMinZ < mZSize) && (iMaxZ >= 0)) {
			if (iMinX < 0) { iMinX = 0; }
			if (iMaxX >= mXSize) { iMaxX = mXSize - 1; }
			if (iMinZ < 0) { iMinZ = 0; }
			if (iMaxZ >= mZSize) { iMaxZ = mZSize - 1; }

			for (int z = iMinZ; z < iMaxZ; ++z) {
				if (z == mZSize - 1) { break; }
				for (int x = iMinX; x < iMaxX; ++x) {
					if (x == mXSize - 1) { continue; }

					glm::vec3 v0(xIndexToX(x), mHeights[z * mXSize + x], zIndexToZ(z));
					glm::vec3 v1(xIndexToX(x+1), mHeights[z * mXSize + (x+1)], zIndexToZ(z));
					glm::vec3 v2(xIndexToX(x), mHeights[(z+1) * mXSize + x], zIndexToZ(z+1));
					glm::vec3 v3(xIndexToX(x+1), mHeights[(z+1) * mXSize + (x+1)], zIndexToZ(z+1));

					std::vector<glm::vec3> triangleVertices1 = { v0, v1, v2 };
					if (checkYAxis(localAABB, triangleVertices1)) {
						triangleColliders.emplace_back(std::make_unique<ConvexPolyhedron>(triangleVertices1));
						triangleColliders.back()->setTransforms(mTransformsMatrix);
					}

					std::vector<glm::vec3> triangleVertices2 = { v1, v3, v2 };
					if (checkYAxis(localAABB, triangleVertices2)) {
						triangleColliders.push_back(std::make_unique<ConvexPolyhedron>(triangleVertices2));
						triangleColliders.back()->setTransforms(mTransformsMatrix);
					}
				}
			}
		}
*/
		return triangleColliders;
	}

// Private functions
	void TerrainCollider::calculateAABB()
	{
		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (int z = 0; z < mZSize; ++z) {
			float zPos = z / static_cast<float>(mZSize - 1) - 0.5f;
			for (int x = 0; x < mXSize; ++x) {
				float xPos = x / static_cast<float>(mXSize - 1) - 0.5f;
				float yPos = mHeights[z * mXSize + x];

				glm::vec3 localPosition(xPos, yPos, zPos);
				glm::vec3 worldPosition = mTransformsMatrix * glm::vec4(localPosition, 1.0f);
				mAABB.minimum = glm::min(mAABB.minimum, worldPosition);
				mAABB.maximum = glm::max(mAABB.maximum, worldPosition);
			}
		}
	}


	AABB TerrainCollider::calculateLocalAABB(const AABB& worldAABB) const
	{
		glm::vec3 lengths = worldAABB.maximum - worldAABB.minimum,
			localMinimum(std::numeric_limits<float>::max()),
			localMaximum(-std::numeric_limits<float>::max());
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				for (int k = 0; k < 2; ++k) {
					glm::vec3 cubePoint = worldAABB.minimum + glm::vec3(i * lengths.x, j * lengths.y, k * lengths.z);
					cubePoint = mInverseTransformsMatrix * glm::vec4(cubePoint, 1.0f);
					localMinimum = glm::min(localMinimum, cubePoint);
					localMaximum = glm::max(localMaximum, cubePoint);
				}
			}
		}

		return { localMinimum, localMaximum };
	}


	bool TerrainCollider::checkYAxis(
		const AABB& aabb, const std::vector<glm::vec3>& vertices
	) const
	{
		float	minY1 = aabb.minimum.y, minY2 = std::numeric_limits<float>::max(),
				maxY1 = aabb.maximum.y, maxY2 = -std::numeric_limits<float>::max();
		for (const glm::vec3& v : vertices) {
			if (v.y < minY2) { minY2 = v.y; }
			if (v.y > maxY2) { maxY2 = v.y; }
		}

		if (minY2 < minY1) {
			std::swap(minY1, minY2);
			std::swap(maxY1, maxY2);
		}

		return (maxY1 >= minY2) && (minY1 <= maxY2);
	}

}}
