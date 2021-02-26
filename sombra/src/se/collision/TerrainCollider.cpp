#include <limits>
#include <cassert>
#include <algorithm>
#include "se/collision/TerrainCollider.h"
#include "se/collision/TriangleCollider.h"

namespace se::collision {

	TerrainCollider::TerrainCollider() :
		mXSize(0), mZSize(0),
		mTransformsMatrix(1.0f), mInverseTransformsMatrix(1.0f),
		mUpdated(true)
	{
		calculateAABB();
	}


	void TerrainCollider::setHeights(
		const std::vector<float>& heights, std::size_t xSize, std::size_t zSize
	) {
		assert(mHeights.size() >= mXSize * mZSize && "There aren't enough heights");

		mHeights = heights;
		mXSize = xSize;
		mZSize = zSize;
		calculateAABB();
	}


	void TerrainCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mInverseTransformsMatrix = glm::inverse(mTransformsMatrix);
		calculateAABB();
		mUpdated = true;
	}


	void TerrainCollider::processOverlapingParts(const AABB& aabb, const ConvexShapeCallback& callback) const
	{
		auto xIndexToX = [this](int i) { return i / static_cast<float>(mXSize - 1) - 0.5f; };
		auto zIndexToZ = [this](int i) { return i / static_cast<float>(mZSize - 1) - 0.5f; };

		// Get the AABB in local coordinates
		AABB localAABB = transform(aabb, mInverseTransformsMatrix);

		// Calculate the indices of the terrain vertices to check with the
		// local position of the AABB
		int iSizeX	= static_cast<int>(mXSize),
			iMinX	= static_cast<int>((iSizeX - 1) * (localAABB.minimum.x + 0.5f)),
			iMaxX	= static_cast<int>(std::ceil((iSizeX - 1) * (localAABB.maximum.x + 0.5f))),
			iSizeZ	= static_cast<int>(mZSize),
			iMinZ	= static_cast<int>((iSizeZ - 1) * (localAABB.minimum.z + 0.5f)),
			iMaxZ	= static_cast<int>(std::ceil((iSizeZ - 1) * (localAABB.maximum.z + 0.5f)));

		if ((iMinX < 0) && (iMaxX >= 0))				{ iMinX = 0; }
		if ((iMaxX >= iSizeX) && (iMinX < iSizeX - 1))	{ iMaxX = iSizeX - 1; }
		if ((iMinZ < 0) && (iMaxZ >= 0))				{ iMinZ = 0; }
		if ((iMaxZ >= iSizeZ) && (iMinZ < iSizeZ - 1))	{ iMaxZ = iSizeZ - 1; }

		if ((iMinX >= 0) && (iMaxX < iSizeZ) && (iMinZ >= 0) && (iMaxZ < iSizeZ)) {
			for (int z = iMinZ; z < iMaxZ; ++z) {
				for (int x = iMinX; x < iMaxX; ++x) {
					glm::vec3 v0(xIndexToX(x), mHeights[z * iSizeX + x], zIndexToZ(z));
					glm::vec3 v1(xIndexToX(x+1), mHeights[z * iSizeX + (x+1)], zIndexToZ(z));
					glm::vec3 v2(xIndexToX(x), mHeights[(z+1) * iSizeX + x], zIndexToZ(z+1));
					glm::vec3 v3(xIndexToX(x+1), mHeights[(z+1) * iSizeX + (x+1)], zIndexToZ(z+1));

					std::array<glm::vec3, 3> triangleVertices1{ v0, v1, v2 };
					if (checkYAxis(localAABB, triangleVertices1)) {
						TriangleCollider trianglePart(triangleVertices1);
						trianglePart.setTransforms(mTransformsMatrix);
						callback(trianglePart);
					}

					std::array<glm::vec3, 3> triangleVertices2{ v1, v3, v2 };
					if (checkYAxis(localAABB, triangleVertices2)) {
						TriangleCollider trianglePart(triangleVertices2);
						trianglePart.setTransforms(mTransformsMatrix);
						callback(trianglePart);
					}
				}
			}
		}
	}

// Private functions
	void TerrainCollider::calculateAABB()
	{
		if ((mXSize == 0) || (mZSize == 0)) {
			mAABB = {};
			return;
		}

		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (std::size_t z = 0; z < mZSize; ++z) {
			float zPos = (mZSize > 1)? z / static_cast<float>(mZSize - 1) - 0.5f : 0.0f;
			for (std::size_t x = 0; x < mXSize; ++x) {
				float xPos = (mXSize > 1)? x / static_cast<float>(mXSize - 1) - 0.5f : 0.0f;
				float yPos = mHeights[z * mXSize + x];

				glm::vec3 localPosition(xPos, yPos, zPos);
				glm::vec3 worldPosition = mTransformsMatrix * glm::vec4(localPosition, 1.0f);
				mAABB.minimum = glm::min(mAABB.minimum, worldPosition);
				mAABB.maximum = glm::max(mAABB.maximum, worldPosition);
			}
		}
	}


	bool TerrainCollider::checkYAxis(
		const AABB& aabb, const std::array<glm::vec3, 3>& vertices
	) const
	{
		float	minY1 = aabb.minimum.y, minY2 = std::numeric_limits<float>::max(),
				maxY1 = aabb.maximum.y, maxY2 =-std::numeric_limits<float>::max();
		for (const glm::vec3& v : vertices) {
			if (v.y < minY2) { minY2 = v.y; }
			if (v.y > maxY2) { maxY2 = v.y; }
		}

		return (maxY1 >= minY2) && (minY1 <= maxY2);
	}

}
