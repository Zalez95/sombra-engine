#include <limits>
#include <algorithm>
#include "se/physics/collision/HalfEdgeMeshExt.h"
#include "se/physics/collision/TerrainCollider.h"
#include "se/physics/collision/TriangleCollider.h"
#include "se/physics/collision/ConvexPolyhedron.h"

namespace se::physics {

	void TerrainCollider::setHeights(
		const float* heights, std::size_t xSize, std::size_t zSize
	) {
		mHeights = std::vector<float>(heights, heights + xSize * zSize);
		mXSize = xSize;
		mZSize = zSize;
		calculateAABB();
		mUpdated = true;
	}

	void TerrainCollider::setPrismHeight(float prismHeight)
	{
		mPrismHeight = prismHeight;
		calculateAABB();
		mUpdated = true;
	}


	void TerrainCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mInverseTransformsMatrix = glm::inverse(mTransformsMatrix);
		calculateAABB();
		mUpdated = true;
	}


	void TerrainCollider::processOverlapingParts(const AABB& aabb, float epsilon, const ConvexShapeCallback& callback) const
	{
		// Get the AABB in local coordinates
		AABB localAABB = transform(aabb, mInverseTransformsMatrix);

		// Calculate the indices of the terrain vertices to check with the
		// local position of the AABB
		int iSizeX	= static_cast<int>(mXSize),
			iMinX	= static_cast<int>((iSizeX - 1) * (localAABB.minimum.x + 0.5f - epsilon)),
			iMaxX	= static_cast<int>(std::ceil((iSizeX - 1) * (localAABB.maximum.x + 0.5f + epsilon))),
			iSizeZ	= static_cast<int>(mZSize),
			iMinZ	= static_cast<int>((iSizeZ - 1) * (localAABB.minimum.z + 0.5f - epsilon)),
			iMaxZ	= static_cast<int>(std::ceil((iSizeZ - 1) * (localAABB.maximum.z + 0.5f + epsilon)));

		if ((iMinX < 0) && (iMaxX >= 0))				{ iMinX = 0; }
		if ((iMaxX >= iSizeX) && (iMinX < iSizeX - 1))	{ iMaxX = iSizeX - 1; }
		if ((iMinZ < 0) && (iMaxZ >= 0))				{ iMinZ = 0; }
		if ((iMaxZ >= iSizeZ) && (iMinZ < iSizeZ - 1))	{ iMaxZ = iSizeZ - 1; }

		if ((iMinX >= 0) && (iMaxX < iSizeX) && (iMinZ >= 0) && (iMaxZ < iSizeZ)) {
			// Check overlaps
			if (mPrismHeight == 0.0f) {
				processTriangles(iMinX, iMinZ, iMaxX, iMaxZ, [&](const TriangleCollider& triangle) {
					const auto& vertices = triangle.getLocalVertices();
					if (checkYAxis(localAABB, vertices.data(), vertices.size(), epsilon)) {
						callback(triangle);
					}
				});
			}
			else {
				processPrisms(iMinX, iMinZ, iMaxX, iMaxZ, [&](const ConvexPolyhedron& prism) {
					const auto& heMesh = prism.getLocalMesh();

					std::array<glm::vec3, 6> vertices;
					for (std::size_t i = 0; i < 6; ++i) {
						vertices[i] = heMesh.vertices[i].location;
					}

					if (checkYAxis(localAABB, vertices.data(), vertices.size(), epsilon)) {
						callback(prism);
					}
				});
			}
		}
	}


	void TerrainCollider::processIntersectingParts(const Ray& ray, float epsilon, const ConvexShapeCallback& callback) const
	{
		struct SearchSquare { std::size_t iMinX = 0, iMinZ = 0, iMaxX = 0, iMaxZ = 0; };

		// Get the ray in local space
		Ray localRay(
			mInverseTransformsMatrix * glm::vec4(ray.origin, 1.0f),
			glm::normalize(mInverseTransformsMatrix * glm::vec4(ray.direction, 0.0f))
		);

		// Search the triangles of the terrain that intersects with ray in
		// local space, spliting the terrain as if it was a Quadtree
		std::vector<SearchSquare> squaresToCheck = { { 0, 0, mXSize - 1, mZSize - 1 } };
		while (!squaresToCheck.empty()) {
			SearchSquare square = squaresToCheck.back();
			squaresToCheck.pop_back();

			// Check if the current AABB intersects with the local ray, we use
			// infinites in the Y direction so we dont have to calculate the
			// maximum of the vertices in the square
			AABB squareAABB = {
				{
					static_cast<float>(square.iMinX) / (mXSize - 1) - 0.5f,
					-std::numeric_limits<float>::max(),
					static_cast<float>(square.iMinZ) / (mZSize - 1) - 0.5f
				},
				{
					static_cast<float>(square.iMaxX) / (mXSize - 1) - 0.5f,
					std::numeric_limits<float>::max(),
					static_cast<float>(square.iMaxZ) / (mZSize - 1) - 0.5f
				}
			};
			if (intersects(squareAABB, localRay, epsilon)) {
				std::size_t sizeX = square.iMaxX - square.iMinX + 1,
							sizeZ = square.iMaxZ - square.iMinZ + 1;
				if ((sizeX > 2) && (sizeZ > 2)) {
					// Split the square and insert its children squares
					std::size_t halfSizeX = sizeX / 2,
								halfSizeZ = sizeZ / 2;
					squaresToCheck.push_back({ square.iMinX, square.iMinZ, square.iMinX + halfSizeX, square.iMinZ + halfSizeZ });
					squaresToCheck.push_back({ square.iMinX + halfSizeX, square.iMinZ, square.iMaxX, square.iMinZ + halfSizeZ });
					squaresToCheck.push_back({ square.iMinX, square.iMinZ + halfSizeZ, square.iMinX + halfSizeX, square.iMaxZ });
					squaresToCheck.push_back({ square.iMinX + halfSizeX, square.iMinZ + halfSizeZ, square.iMaxX, square.iMaxZ });
				}
				else {
					// Check intersections
					if (mPrismHeight == 0.0f) {
						processTriangles(square.iMinX, square.iMinZ, square.iMaxX, square.iMaxZ, [&](const TriangleCollider& triangle) {
							if (intersects(triangle.getAABB(), ray, epsilon)) {
								callback(triangle);
							}
						});
					}
					else {
						processPrisms(square.iMinX, square.iMinZ, square.iMaxX, square.iMaxZ, [&](const ConvexPolyhedron& prism) {
							if (intersects(prism.getAABB(), ray, epsilon)) {
								callback(prism);
							}
						});
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

		float inverseXMinus1 = 1.0f / static_cast<float>(mXSize - 1);
		float inverseZMinus1 = 1.0f / static_cast<float>(mZSize - 1);

		mAABB = { glm::vec3( std::numeric_limits<float>::max()), glm::vec3(-std::numeric_limits<float>::max()) };
		for (std::size_t z = 0; z < mZSize; ++z) {
			float zPos = z * inverseZMinus1 - 0.5f;
			for (std::size_t x = 0; x < mXSize; ++x) {
				float xPos = x * inverseXMinus1 - 0.5f;
				float yPos = mHeights[z * mXSize + x];

				glm::vec3 localPosition(xPos, yPos, zPos);
				glm::vec3 worldPosition = mTransformsMatrix * glm::vec4(localPosition, 1.0f);
				mAABB.minimum = glm::min(mAABB.minimum, worldPosition);
				mAABB.maximum = glm::max(mAABB.maximum, worldPosition);

				if (mPrismHeight != 0.0f) {
					glm::vec3 localPositionP(xPos, yPos - mPrismHeight, zPos);
					glm::vec3 worldPositionP = mTransformsMatrix * glm::vec4(localPositionP, 1.0f);
					mAABB.minimum = glm::min(mAABB.minimum, worldPositionP);
					mAABB.maximum = glm::max(mAABB.maximum, worldPositionP);
				}
			}
		}
	}


	void TerrainCollider::processTriangles(
		std::size_t iMinX, std::size_t iMinZ, std::size_t iMaxX, std::size_t iMaxZ,
		const TriangleCallback& callback
	) const
	{
		float inverseXMinus1 = 1.0f / static_cast<float>(mXSize - 1);
		float inverseZMinus1 = 1.0f / static_cast<float>(mZSize - 1);

		for (std::size_t z = iMinZ; z < iMaxZ; ++z) {
			for (std::size_t x = iMinX; x < iMaxX; ++x) {
				glm::vec3 v0(x * inverseXMinus1 - 0.5f, mHeights[z * mXSize + x], z * inverseZMinus1 - 0.5f);
				glm::vec3 v1((x+1) * inverseXMinus1 - 0.5f, mHeights[z * mXSize + (x+1)], z * inverseZMinus1 - 0.5f);
				glm::vec3 v2(x * inverseXMinus1 - 0.5f, mHeights[(z+1) * mXSize + x], (z+1) * inverseZMinus1 - 0.5f);
				glm::vec3 v3((x+1) * inverseXMinus1 - 0.5f, mHeights[(z+1) * mXSize + (x+1)], (z+1) * inverseZMinus1 - 0.5f);

				TriangleCollider trianglePart1({ v0, v2, v1 });
				trianglePart1.setTransforms(mTransformsMatrix);
				callback(trianglePart1);

				TriangleCollider trianglePart2({ v1, v2, v3 });
				trianglePart2.setTransforms(mTransformsMatrix);
				callback(trianglePart2);
			}
		}
	}


	void TerrainCollider::processPrisms(
		std::size_t iMinX, std::size_t iMinZ, std::size_t iMaxX, std::size_t iMaxZ,
		const PrismCallback& callback
	) const
	{
		float inverseXMinus1 = 1.0f / static_cast<float>(mXSize - 1);
		float inverseZMinus1 = 1.0f / static_cast<float>(mZSize - 1);

		for (std::size_t z = iMinZ; z < iMaxZ; ++z) {
			for (std::size_t x = iMinX; x < iMaxX; ++x) {
				glm::vec3 v0(x * inverseXMinus1 - 0.5f, mHeights[z * mXSize + x], z * inverseZMinus1 - 0.5f);
				glm::vec3 v1((x+1) * inverseXMinus1 - 0.5f, mHeights[z * mXSize + (x+1)], z * inverseZMinus1 - 0.5f);
				glm::vec3 v2(x * inverseXMinus1 - 0.5f, mHeights[(z+1) * mXSize + x], (z+1) * inverseZMinus1 - 0.5f);
				glm::vec3 v3((x+1) * inverseXMinus1 - 0.5f, mHeights[(z+1) * mXSize + (x+1)], (z+1) * inverseZMinus1 - 0.5f);

				ConvexPolyhedron prism1( createPrism({ v0, v2, v1 }, mPrismHeight) );
				prism1.setTransforms(mTransformsMatrix);
				callback(prism1);

				ConvexPolyhedron prism2( createPrism({ v1, v2, v3 }, mPrismHeight) );
				prism2.setTransforms(mTransformsMatrix);
				callback(prism2);
			}
		}
	}


	bool TerrainCollider::checkYAxis(
		const AABB& aabb, const glm::vec3* vertices, std::size_t vertexCount, float epsilon
	) const
	{
		float	minY1 = aabb.minimum.y, minY2 = std::numeric_limits<float>::max(),
				maxY1 = aabb.maximum.y, maxY2 =-std::numeric_limits<float>::max();
		for (std::size_t i = 0; i < vertexCount; ++i) {
			minY2 = std::min(minY2, vertices[i].y);
			maxY2 = std::max(maxY2, vertices[i].y);
		}

		return (maxY1 + epsilon >= minY2) && (minY1 - epsilon <= maxY2);
	}

}
