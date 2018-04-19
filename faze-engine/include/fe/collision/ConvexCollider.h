#ifndef CONVEX_COLLIDER_H
#define CONVEX_COLLIDER_H

#include "Collider.h"

namespace fe { namespace collision {

	/**
	 * Class ConvexCollider, it's a Collider with a Convex shape.
	 */
	class ConvexCollider : public Collider
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ConvexCollider() {};

		/** Updates the scale, translation and orientation of the ConvexCollider
		 * with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the ConvexCollider */
		virtual void setTransforms(const glm::mat4& transforms) = 0;

		/** @return	the transformations matrix currently applied to the
		 *			ConvexCollider */
		virtual glm::mat4 getTransforms() const = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			ConvexCollider */
		virtual AABB getAABB() const = 0;

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

}}

#endif		// CONVEX_COLLIDER_H
