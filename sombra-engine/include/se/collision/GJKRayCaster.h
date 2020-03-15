#ifndef GJK_RAY_CASTER_H
#define GJK_RAY_CASTER_H

#include <utility>
#include "RayCast.h"

namespace se::collision {

	class ConvexCollider;


	/**
	 * Class GJKRayCaster, it's the class used for detecting ray hits with
	 * arbitrary convex shapes with the GJK (Gilbert–Johnson–Keerthi)
	 * Algorithm.
	 *
	 * @see "Ray Casting against General Convex Objects with Application to
	 * Continuous CollisionDetection" by Gino Van Den Bergen
	 * @note The algorithm implemented only works for 3 dimensions or less.
	 */
	class GJKRayCaster
	{
	private:	// Attributes
		/** The precision of the comparisons in the GJK algorithm */
		const float mEpsilon;

	public:		// Functions
		/** Creates a new GJKRayCaster
		 *
		 * @param	epsilon the comparison precision of the algorithm */
		GJKRayCaster(float epsilon) : mEpsilon(epsilon) {};

		/** Checks if the given ray intersects the given collider
		 *
		 * @param	rayOrigin the origin point of the ray
		 * @param	rayDirection the direction of the ray
		 * @param	collider the collider to test
		 * @return	a pair with a boolean that tell if the ray intersects the
		 *			collider and the ray cast data */
		std::pair<bool, RayCast> calculateRayCast(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
			const ConvexCollider& collider
		) const;
	};

}

#endif		// GJK_RAY_CASTER_H
