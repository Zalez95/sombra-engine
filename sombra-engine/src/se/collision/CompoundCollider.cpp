#include <limits>
#include "se/collision/CompoundCollider.h"

namespace se::collision {

	CompoundCollider::CompoundCollider(const std::vector<ConvexPolyhedron>& convexParts) :
		mConvexParts(convexParts), mTransformsMatrix(1.0f)
	{
		calculateAABB();
	}


	void CompoundCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (ConvexPolyhedron& convexPart : mConvexParts) {
			convexPart.setTransforms(transforms);
		}

		calculateAABB();
	}


	std::vector<const ConvexCollider*> CompoundCollider::getOverlapingParts(const AABB& aabb) const
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
	void CompoundCollider::calculateAABB()
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

}
