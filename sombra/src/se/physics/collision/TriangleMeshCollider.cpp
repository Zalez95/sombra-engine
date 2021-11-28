#include "se/physics/collision/TriangleMeshCollider.h"
#include "se/physics/collision/TriangleCollider.h"
#include "AABBAVLTree.h"

namespace se::physics {

	TriangleMeshCollider::TriangleMeshCollider(
		const glm::vec3* vertices, std::size_t numVertices,
		const unsigned short* indices, std::size_t numIndices
	) : mVertices(vertices, vertices + numVertices),
		mIndices(indices, indices + numIndices),
		mTransformsMatrix(1.0f)
	{
		calculateAABBTree();
	}


	TriangleMeshCollider::TriangleMeshCollider(const TriangleMeshCollider& other) :
		ConcaveCollider(other), mVertices(other.mVertices), mIndices(other.mIndices),
		mAABBTree( std::make_unique<AABBAVLTree<unsigned short>>(*other.mAABBTree) ),
		mTransformsMatrix(other.mTransformsMatrix) {}


	TriangleMeshCollider::~TriangleMeshCollider() {}


	TriangleMeshCollider& TriangleMeshCollider::operator=(const TriangleMeshCollider& other)
	{
		ConcaveCollider::operator=(other);
		mVertices = other.mVertices;
		mIndices = other.mIndices;
		mAABBTree = std::make_unique<AABBAVLTree<unsigned short>>(*other.mAABBTree);
		mTransformsMatrix = other.mTransformsMatrix;
		return *this;
	}


	void TriangleMeshCollider::setMesh(
		const glm::vec3* vertices, std::size_t numVertices,
		const unsigned short* indices, std::size_t numIndices
	) {
		mVertices = std::vector<glm::vec3>(vertices, vertices + numVertices);
		mIndices = std::vector<unsigned short>(indices, indices + numIndices);
		calculateAABBTree();
		mUpdated = true;
	}


	void TriangleMeshCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		calculateAABBTree();
		mUpdated = true;
	}


	AABB TriangleMeshCollider::getAABB() const
	{
		if (mAABBTree && (mAABBTree->getNumNodes() > 0)) {
			return mAABBTree->getRootNodeAABB();
		}
		return AABB();
	}


	void TriangleMeshCollider::processOverlapingParts(const AABB& aabb, float epsilon, const ConvexShapeCallback& callback) const
	{
		mAABBTree->calculateOverlapsWith(aabb, epsilon, [&](std::size_t nodeId) {
			unsigned short triIndex = mAABBTree->getNodeUserData(nodeId);
			TriangleCollider collider = getTriangleCollider(triIndex);
			callback(collider);
		});
	}


	void TriangleMeshCollider::processIntersectingParts(const Ray& ray, float epsilon, const ConvexShapeCallback& callback) const
	{
		mAABBTree->calculateIntersectionsWith(ray, epsilon, [&](std::size_t nodeId) {
			unsigned short triIndex = mAABBTree->getNodeUserData(nodeId);
			TriangleCollider collider = getTriangleCollider(triIndex);
			callback(collider);
		});
	}

// Private functions
	void TriangleMeshCollider::calculateAABBTree()
	{
		mAABBTree = std::make_unique<AABBAVLTree<unsigned short>>();

		for (unsigned short triIndex = 0; triIndex < mIndices.size() / 3; ++triIndex) {
			mAABBTree->addNode(getTriangleCollider(triIndex).getAABB(), triIndex);
		}
	}


	TriangleCollider TriangleMeshCollider::getTriangleCollider(unsigned short triangleIndex) const
	{
		TriangleCollider collider({
			mVertices[ mIndices[3 * triangleIndex] ],
			mVertices[ mIndices[3 * triangleIndex + 1] ],
			mVertices[ mIndices[3 * triangleIndex + 2] ]
		});
		collider.setTransforms(mTransformsMatrix);

		return collider;
	}

}
