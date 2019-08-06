#ifndef CONVEX_COLLIDER_H
#define CONVEX_COLLIDER_H

#include "Collider.h"

namespace se::collision {

	/**
	 * Class ConvexCollider, it's a Collider with a Convex shape.
	 */
	class ConvexCollider : public Collider
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ConvexCollider() = default;

		/** Updates the scale, translation and orientation of the ConvexCollider
		 * with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the ConvexCollider */
		void setTransforms(const glm::mat4& transforms) override = 0;

		/** @return	the transformations matrix currently applied to the
		 *			ConvexCollider */
		glm::mat4 getTransforms() const override = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			ConvexCollider */
		AABB getAABB() const override = 0;

		/** @return	true if the ConvexCollider has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		virtual bool updated() const override = 0;

		/** Resets the updated state of the ConvexCollider */
		virtual void resetUpdatedState() override = 0;

		/** Calculates the coordinates of the ConvexCollider's furthest point
		 * in the given direction
		 *
		 * @param	direction the direction towards we want to get the furthest
		 *			point
		 * @param	pointWorld the vector where we are going to store the
		 *			coordinates in world space of ConvexCollider's furthest
		 *			point
		 * @param	pointLocal the vector where we are going to store the
		 *			coordinates in local space of ConvexCollider's furthest
		 *			point */
		virtual void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const = 0;
	};

}

#endif		// CONVEX_COLLIDER_H
