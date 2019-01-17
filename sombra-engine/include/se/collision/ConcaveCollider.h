#ifndef CONCAVE_COLLIDER_H
#define CONCAVE_COLLIDER_H

#include <vector>
#include <memory>
#include "Collider.h"

namespace se::collision {

	class ConvexCollider;


	/**
	 * Class ConcaveCollider, it's a Collider with a Concave shape.
	 */
	class ConcaveCollider : public Collider
	{
	public:		// Nested types
		using ConvexColliderSPtr = std::shared_ptr<ConvexCollider>;

	public:		// Functions
		/** Class destructor */
		virtual ~ConcaveCollider() = default;

		/** Updates the scale, translation and orientation of the
		 * ConcaveCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the ConcaveCollider */
		void setTransforms(const glm::mat4& transforms) override = 0;

		/** @return	the transformations matrix currently applied to the
		 *			ConcaveCollider */
		glm::mat4 getTransforms() const override = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			ConcaveCollider */
		AABB getAABB() const override = 0;

		/** Calculates the posible overlaping parts of the Collider with the
		 * given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	the pointers to the Convex parts of the collider that could
		 *			be overlaping with the given AABB */
		virtual std::vector<ConvexColliderSPtr> getOverlapingParts(
			const AABB& aabb
		) const = 0;
	};

}

#endif		// CONCAVE_COLLIDER_H
