#ifndef CONVEX_COLLIDER_H
#define CONVEX_COLLIDER_H

#include "Collider.h"

namespace se::physics {

	/**
	 * Class ConvexCollider, it's a Collider with a Convex shape.
	 */
	class ConvexCollider : public Collider
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ConvexCollider() = default;

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
