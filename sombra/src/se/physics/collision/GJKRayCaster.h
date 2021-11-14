#ifndef GJK_RAY_CASTER_H
#define GJK_RAY_CASTER_H

#include "se/physics/collision/Ray.h"
#include "Simplex.h"

namespace se::physics {

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

		/** The maximum number of iterations of the algorithm */
		const std::size_t mMaxIterations;

	public:		// Functions
		/** Creates a new GJKRayCaster
		 *
		 * @param	epsilon the comparison precision of the algorithm
		 * @param	maxIterations the maximum number of iterations of the
		 *			algorithm */
		GJKRayCaster(float epsilon, std::size_t maxIterations) :
			mEpsilon(epsilon), mMaxIterations(maxIterations) {};

		/** Checks if the given ray intersects the given collider
		 *
		 * @param	ray the ray to test
		 * @param	collider the collider to test
		 * @return	a pair with a boolean that tell if the ray intersects the
		 *			collider and the ray cast data */
		std::pair<bool, RayHit> calculateRayCast(
			const Ray& ray, const ConvexCollider& collider
		) const;
	private:
		/** Calculates the closest point to the origin of coordinates in CSO
		 * space in the given simplex
		 *
		 * @param	simplex the simplex to calculate its closest point to the
		 *			origin
		 * @param	closestPoints a vector of bools that tells which vertices
		 *			of the simplex were used for calculating the closest point
		 *			to the origin
		 * @return	a pair with a boolean that tells if the SupportPoint was
		 *			calculated successfully and the closest SupportPoint to
		 *			the origin */
		std::pair<bool, SupportPoint> calculateClosestPoint(
			const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
		) const;

		/** Implements @see calculateClosestPoint with a simplex of 1 vertex
		 *
		 * @param	simplex the simplex to calculate its closest point to the
		 *			origin
		 * @param	closestPoints a vector of bools that tells which vertices
		 *			of the simplex were used for calculating the closest point
		 *			to the origin
		 * @return	a pair with a boolean that tells if the SupportPoint was
		 *			calculated successfully and the closest SupportPoint to
		 *			the origin */
		std::pair<bool, SupportPoint> calculateClosestPoint1(
			const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
		) const;

		/** Implements @see calculateClosestPoint with a simplex of 2 vertices
		 *
		 * @param	simplex the simplex to calculate its closest point to the
		 *			origin
		 * @param	closestPoints a vector of bools that tells which vertices
		 *			of the simplex were used for calculating the closest point
		 *			to the origin
		 * @return	a pair with a boolean that tells if the SupportPoint was
		 *			calculated successfully and the closest SupportPoint to
		 *			the origin */
		std::pair<bool, SupportPoint> calculateClosestPoint2(
			const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
		) const;

		/** Implements @see calculateClosestPoint with a simplex of 3 vertices
		 *
		 * @param	simplex the simplex to calculate its closest point to the
		 *			origin
		 * @param	closestPoints a vector of bools that tells which vertices
		 *			of the simplex were used for calculating the closest point
		 *			to the origin
		 * @return	a pair with a boolean that tells if the SupportPoint was
		 *			calculated successfully and the closest SupportPoint to
		 *			the origin */
		std::pair<bool, SupportPoint> calculateClosestPoint3(
			const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
		) const;

		/** Implements @see calculateClosestPoint with a simplex of 4 vertices
		 *
		 * @param	simplex the simplex to calculate its closest point to the
		 *			origin
		 * @param	closestPoints a vector of bools that tells which vertices
		 *			of the simplex were used for calculating the closest point
		 *			to the origin
		 * @return	a pair with a boolean that tells if the SupportPoint was
		 *			calculated successfully and the closest SupportPoint to
		 *			the origin */
		std::pair<bool, SupportPoint> calculateClosestPoint4(
			const Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
		) const;

		/** Removes the simplex points that arent being used for calculating the
		 * closest distance to the origin
		 *
		 * @param	simplex the simplex to remove its points
		 * @param	closestPoints a vector of bools that tells which vertices
		 *			of the simplex were used for calculating the closest point
		 *			to the origin */
		static void reduce(
			Simplex& simplex, utils::FixedVector<bool, 4>& closestPoints
		);
	};

}

#endif		// GJK_RAY_CASTER_H
