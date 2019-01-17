#ifndef COMPOSITE_COLLIDER_H
#define COMPOSITE_COLLIDER_H

#include "ConcaveCollider.h"

namespace se::collision {

	/**
	 * Class CompositeCollider, it's a Collider with a concave shape stored as
	 * a hierarchy of child Colliders following the Composite pattern
	 */
	class CompositeCollider : public ConcaveCollider
	{
	private:	// Nested types
		using ColliderUPtr = std::unique_ptr<Collider>;
		using ColliderSPtr = std::shared_ptr<Collider>;

	private:	// Attributes
		/** The multiple pieces in which the CompositeCollider is splited */
		std::vector<ColliderSPtr> mParts;

		/** The transformation matrix of the CompositeCollider */
		glm::mat4 mTransformsMatrix;

		/** The AABB of the CompositeCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new CompositeCollider located at the origin of coordinates
		 * from the given convex pieces
		 *
		 * @param	parts the multiple colliders in which the CompositeCollider
		 *			is splitted */
		CompositeCollider(std::vector<ColliderUPtr>& parts);

		/** Updates the scale, translation and orientation of the
		 * CompositeCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the
		 *			CompositeCollider */
		void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			CompositeCollider */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			CompositeCollider */
		AABB getAABB() const override { return mAABB; };

		/** Calculates the posible overlaping parts of the Collider with the
		 * given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	the pointers to the Convex parts of the collider that could
		 *			be overlaping with the given AABB */
		std::vector<ConvexColliderSPtr> getOverlapingParts(
			const AABB& aabb
		) const override;
	private:
		/** Calculates the AABB of the CompositeCollider with its Convex
		 * parts */
		void calculateAABB();
	};

}

#endif		// COMPOSITE_COLLIDER_H
