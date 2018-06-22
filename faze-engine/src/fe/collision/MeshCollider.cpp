#include <limits>
#include "fe/collision/MeshCollider.h"
#include "QuickHull.h"

namespace fe { namespace collision {

	MeshCollider::MeshCollider(const HalfEdgeMesh& meshData, ConvexStrategy strategy) :
		mTransformsMatrix(1.0f)
	{
		switch (strategy) {
			case ConvexStrategy::QuickHull: {
					mConvexParts.push_back(QuickHull().calculate(meshData));
				} break;
			case ConvexStrategy::HACD:
				throw std::runtime_error("Not implemented");
				break;
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

}}
