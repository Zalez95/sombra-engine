#include <limits>
#include "se/collision/CompositeCollider.h"
#include "se/collision/ConvexCollider.h"

namespace se::collision {

	CompositeCollider::CompositeCollider(std::vector<ColliderUPtr> parts) :
		mParts(std::move(parts)), mTransformsMatrix(1.0f), mUpdated(true)
	{
		calculateAABB();
	}


	CompositeCollider::CompositeCollider(const CompositeCollider& other)
	{
		for (const ColliderUPtr& part : other.mParts) {
			mParts.emplace_back( part->clone() );
		}
		mTransformsMatrix = other.mTransformsMatrix;
		mAABB = other.mAABB;
		mUpdated = other.mUpdated;
	}


	CompositeCollider& CompositeCollider::operator=(const CompositeCollider& other)
	{
		mParts.clear();
		for (const ColliderUPtr& part : other.mParts) {
			mParts.emplace_back( part->clone() );
		}
		mTransformsMatrix = other.mTransformsMatrix;
		mAABB = other.mAABB;
		mUpdated = other.mUpdated;

		return *this;
	}


	void CompositeCollider::setTransforms(const glm::mat4& transforms)
	{
		mTransformsMatrix = transforms;
		mUpdated = true;

		for (ColliderUPtr& part : mParts) {
			part->setTransforms(transforms);
		}

		calculateAABB();
	}


	bool CompositeCollider::updated() const
	{
		bool ret = mUpdated;

		for (const ColliderUPtr& part : mParts) {
			ret |= part->updated();
		}

		return ret;
	}


	CompositeCollider& CompositeCollider::addPart(ColliderUPtr part)
	{
		mParts.emplace_back(std::move(part));
		calculateAABB();
		mUpdated = true;

		return *this;
	}


	void CompositeCollider::resetUpdatedState()
	{
		mUpdated = false;
		for (ColliderUPtr& part : mParts) {
			part->resetUpdatedState();
		}
	}


	void CompositeCollider::processOverlapingParts(const AABB& aabb, const ConvexShapeCallback& callback) const
	{
		for (const ColliderUPtr& part : mParts) {
			if (overlaps(aabb, part->getAABB())) {
				if (auto convexPart = dynamic_cast<const ConvexCollider*>(part.get())) {
					callback(*convexPart);
				}
				else if (auto concavePart = dynamic_cast<const ConcaveCollider*>(part.get())) {
					concavePart->processOverlapingParts(aabb, callback);
				}
			}
		}
	}


	CompositeCollider& CompositeCollider::removePart(Collider* part)
	{
		mParts.erase(
			std::remove_if(mParts.begin(), mParts.end(), [&](const ColliderUPtr& p) { return p.get() == part; }),
			mParts.end()
		);
		calculateAABB();
		mUpdated = true;

		return *this;
	}

// Private functions
	void CompositeCollider::calculateAABB()
	{
		mAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (ColliderUPtr& part : mParts) {
			AABB partAABB = part->getAABB();
			mAABB.minimum = glm::min(mAABB.minimum, partAABB.minimum);
			mAABB.maximum = glm::max(mAABB.maximum, partAABB.maximum);
		}
	}

}
