#ifndef CONCAVE_COLLIDER_H
#define CONCAVE_COLLIDER_H

#include <functional>
#include "Collider.h"
#include "Ray.h"

namespace se::physics {

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

		/** Calls the given callback for each of the overlaping convex parts of
		 * the ConcaveCollider with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @param	epsilon the epsilon value used for AABB overlap checks
		 * @param	callback the function to call */
		virtual void processOverlapingParts(
			const AABB& aabb, float epsilon, const ConvexShapeCallback& callback
		) const = 0;

		/** Calls the given callback for each of the intersecting convex parts
		 * of the ConcaveCollider with the given ray
		 *
		 * @param	ray the ray to test
		 * @param	epsilon the epsilon value used for ray intersection checks
		 * @param	callback the function to call */
		virtual void processIntersectingParts(
			const Ray& ray, float epsilon, const ConvexShapeCallback& callback
		) const = 0;
	};

}

#endif		// CONCAVE_COLLIDER_H
