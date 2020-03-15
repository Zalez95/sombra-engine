#ifndef CONCAVE_COLLIDER_H
#define CONCAVE_COLLIDER_H

#include <functional>
#include "Collider.h"

namespace se::collision {

	class ConvexCollider;


	/**
	 * Class ConcaveCollider, it's a Collider with a Concave shape.
	 */
	class ConcaveCollider : public Collider
	{
	public:		// Nested types
		using ConvexShapeCallback = std::function<void(const ConvexCollider&)>;

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

		/** @return	true if the ConcaveCollider has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		virtual bool updated() const override = 0;

		/** Resets the updated state of the ConcaveCollider */
		virtual void resetUpdatedState() override = 0;

		/** Calls the given callback for each of the overlaping convex parts of
		 * the ConcaveCollider with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @param	callback the function to call */
		virtual void processOverlapingParts(
			const AABB& aabb, const ConvexShapeCallback& callback
		) const = 0;
	};

}

#endif		// CONCAVE_COLLIDER_H
