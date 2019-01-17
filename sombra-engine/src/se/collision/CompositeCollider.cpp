#include <limits>
#include "se/collision/CompositeCollider.h"
#include "se/collision/ConvexCollider.h"

namespace se::collision {

	CompositeCollider::CompositeCollider(std::vector<ColliderUPtr>& parts) :
		mTransformsMatrix(1.0f)
	{
		for (ColliderUPtr& part : parts) {
			mParts.push_back(std::move(part));
		}

		calculateAABB();
	}


	void CompositeCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;

		for (ColliderSPtr& part : mParts) {
			part->setTransforms(transforms);
		}

		calculateAABB();
	}


	std::vector<CompositeCollider::ConvexColliderSPtr> CompositeCollider::getOverlapingParts(
		const AABB& aabb
	) const
	{
		using ConcaveColliderSPtr = std::shared_ptr<ConcaveCollider>;
		std::vector<ConvexColliderSPtr> overlapingParts;

		for (const ColliderSPtr& part : mParts) {
			if (overlaps(aabb, part->getAABB())) {
				if (ConvexColliderSPtr convexPart = std::dynamic_pointer_cast<ConvexCollider>(part)) {
					overlapingParts.push_back(convexPart);
				}
				else if (ConcaveColliderSPtr concavePart = std::dynamic_pointer_cast<ConcaveCollider>(part)) {
					auto overlapingParts2 = concavePart->getOverlapingParts(aabb);
					overlapingParts.insert(overlapingParts.end(), overlapingParts2.begin(), overlapingParts2.end());
				}
			}
		}

		return overlapingParts;
	}

// Private functions
	void CompositeCollider::calculateAABB()
	{
		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (ColliderSPtr& part : mParts) {
			AABB partAABB = part->getAABB();
			mAABB.minimum = glm::min(mAABB.minimum, partAABB.minimum);
			mAABB.maximum = glm::max(mAABB.maximum, partAABB.maximum);
		}
	}

}
