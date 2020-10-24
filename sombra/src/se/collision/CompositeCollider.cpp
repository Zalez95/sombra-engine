#include <limits>
#include "se/collision/CompositeCollider.h"
#include "se/collision/ConvexCollider.h"

namespace se::collision {

	CompositeCollider::CompositeCollider(std::vector<ColliderUPtr>& parts) :
		mTransformsMatrix(1.0f)
	{
		for (ColliderUPtr& part : parts) {
			mParts.emplace_back(std::move(part));
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


	bool CompositeCollider::updated() const
	{
		bool ret = false;

		for (const ColliderSPtr& part : mParts) {
			if (part->updated()) {
				ret = true;
			}
		}

		return ret;
	}


	void CompositeCollider::resetUpdatedState()
	{
		for (ColliderSPtr& part : mParts) {
			part->resetUpdatedState();
		}
	}


	void CompositeCollider::processOverlapingParts(const AABB& aabb, const ConvexShapeCallback& callback) const
	{
		for (const ColliderSPtr& part : mParts) {
			if (overlaps(aabb, part->getAABB())) {
				if (auto convexPart = std::dynamic_pointer_cast<ConvexCollider>(part)) {
					callback(*convexPart);
				}
				else if (auto concavePart = std::dynamic_pointer_cast<ConcaveCollider>(part)) {
					concavePart->processOverlapingParts(aabb, callback);
				}
			}
		}
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
