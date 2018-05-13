#include <limits>
#include <cassert>
#include "fe/collision/MeshCollider.h"
#include "fe/collision/ConvexPolyhedron.h"

namespace fe { namespace collision {

	MeshCollider::MeshCollider(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices
	) : mVertices(vertices), mVerticesWorld(vertices), mIndices(indices),
		mTransformsMatrix(1.0f)
	{
		assert(mIndices.size() % 3 == 0 && "The faces of the MeshCollider must be triangles");

		calculateAABBs();
	}


	MeshCollider::MeshCollider(
		const std::vector<glm::vec3>& vertices,
		const std::vector<unsigned short>& indices,
		const glm::mat4& transforms
	) : mVertices(vertices), mVerticesWorld(vertices.size()), mIndices(indices)
	{
		assert(mIndices.size() % 3 == 0 && "The faces of the MeshCollider must be triangles");

		setTransforms(transforms);
	}


	void MeshCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (size_t i = 0; i < mVertices.size(); ++i) {
			mVerticesWorld[i] = transforms * glm::vec4(mVertices[i], 1.0f);
		}

		calculateAABBs();
	}


	std::vector<ConcaveCollider::ConvexPart> MeshCollider::getOverlapingParts(const AABB& aabb) const
	{
		std::vector<ConvexPart> triangleColliders;

		for (size_t i = 0; i < mTriangleAABBs.size(); ++i) {
			if (overlaps(aabb, mTriangleAABBs[i])) {
				std::vector<glm::vec3> triangleVertices;
				triangleVertices.reserve(3);
				for (int j = 0; j < 3; ++j) {
					triangleVertices.push_back( mVertices[mIndices[3*i + j]] );
				}

				triangleColliders.push_back(std::make_unique<ConvexPolyhedron>(triangleVertices));
				triangleColliders.back()->setTransforms(mTransformsMatrix);
			}
		}

		return triangleColliders;
	}

// Private functions
	void MeshCollider::calculateAABBs()
	{
		int nTriangles = mIndices.size() / 3;

		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};
		mTriangleAABBs = std::vector<AABB>(nTriangles, mAABB);

		for (int i = 0; i < nTriangles; ++i) {
			for (int j = 0; j < 3; ++j) {
				const glm::vec3& vertex = mVerticesWorld[ mIndices[3*i + j] ];

				mTriangleAABBs[i].minimum = glm::min(mTriangleAABBs[i].minimum, vertex);
				mTriangleAABBs[i].maximum = glm::max(mTriangleAABBs[i].maximum, vertex);
				mAABB.minimum = glm::min(mAABB.minimum, vertex);
				mAABB.maximum = glm::max(mAABB.maximum, vertex);
			}
		}
	}

}}
