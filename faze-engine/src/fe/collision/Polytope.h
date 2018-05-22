#ifndef POLYTOPE_H
#define POLYTOPE_H

#include <deque>
#include <vector>
#include "fe/collision/SupportPoint.h"
#include "Triangle.h"

namespace fe { namespace collision {

	class ConvexCollider;


	/**
	 * Struct Polytope
	 */
	struct Polytope
	{
		/** The vertices of the Polytope
		 * @note	we use deque instead of vectors to avoid reference
		 *			invalidations when pushing back SupportPoints */
		std::deque<SupportPoint> vertices;

		/** The faces of the Polytope */
		std::vector<Triangle> faces;

		/** Creates a new Polytope from the given simplex points by expanding it
		 *
		 * @param	collider1 the first of the colliders needed for creating
		 *			the polytope
		 * @param	collider2 the second of the colliders needed for creating
		 *			the polytope
		 * @param	simplex a vector with the initial simplex points */
		Polytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<SupportPoint>& simplex
		);
	};

}}

#endif		// POLYTOPE_H
