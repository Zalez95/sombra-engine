#include "MeshCollider.h"
#include <limits>
#include <cassert>
#include "ConvexPolyhedron.h"

namespace collision {

	MeshCollider::MeshCollider(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices
	) : mVertices(vertices), mVerticesWorld(vertices), mIndices(indices)
	{
		assert(mIndices.size() % 3 == 0 && "The faces of the MeshCollider must be triangles");

		calculateAABBs();
	}
	
	
	void MeshCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		
		for (size_t i = 0; i < mVertices.size(); ++i) {
			mVerticesWorld[i] = glm::vec3(transforms * glm::vec4(mVertices[i], 1.0f));
		}

		calculateAABBs();
	}


	std::vector<MeshCollider::ConvexPart> MeshCollider::getOverlapingParts(const AABB& aabb) const
	{
		std::vector<ConvexPart> triangleColliders;

		for (size_t i = 0; i < mTriangleAABBs.size(); ++i) {
			const AABB* b1 = &mTriangleAABBs[i];
			const AABB* b2 = &aabb;
			
			if (b2->mMinimum.x < b1->mMinimum.x) { std::swap(b1, b2); }
			bool intersecX = (b1->mMaximum.x > b2->mMinimum.x) && (b1->mMinimum.x < b2->mMaximum.x);

			if (b2->mMinimum.y < b1->mMinimum.y) { std::swap(b1, b2); }
			bool intersecY = (b1->mMaximum.y > b2->mMinimum.y) && (b1->mMinimum.y < b2->mMaximum.y);

			if (b2->mMinimum.z < b1->mMinimum.z) { std::swap(b1, b2); }
			bool intersecZ = (b1->mMaximum.z > b2->mMinimum.z) && (b1->mMinimum.z < b2->mMaximum.z);

			if (intersecX && intersecY && intersecZ) {
				std::vector<glm::vec3> triangleVertices;
				triangleVertices.reserve(3);
				for (size_t j = 0; j < 3; ++j) {
					triangleVertices.push_back( mVerticesWorld[mIndices[3*i + j]] );
				}

				triangleColliders.push_back(std::make_unique<ConvexPolyhedron>(triangleVertices));
			}
		}

		return triangleColliders;
	}

// Private functions
	void MeshCollider::calculateAABBs()
	{
		size_t nTriangles = mIndices.size() / 3;

		mAABB = AABB(
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		);
		mTriangleAABBs = std::vector<AABB>(nTriangles, mAABB);

		for (size_t i = 0; i < nTriangles; ++i) {
			for (size_t j = 0; j < 3; ++j) {
				const glm::vec3& vertex = mVerticesWorld[ mIndices[3*i + j] ];

				mTriangleAABBs[i].mMinimum = glm::min(mTriangleAABBs[i].mMinimum, vertex);
				mTriangleAABBs[i].mMaximum = glm::max(mTriangleAABBs[i].mMaximum, vertex);
				mAABB.mMinimum = glm::min(mAABB.mMinimum, vertex);
				mAABB.mMaximum = glm::max(mAABB.mMaximum, vertex);
			}
		}
	}

}
