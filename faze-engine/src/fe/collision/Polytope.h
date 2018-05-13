#ifndef POLYTOPE_H
#define POLYTOPE_H

#include <list>
#include <vector>
#include "Triangle.h"

namespace fe { namespace collision {

	class ConvexCollider;


	/**
	 * Struct Polytope
	 */
	struct Polytope
	{
		/** The vertices of the Polytope */
		std::list<SupportPoint> vertices;

		/** The faces of the Polytope */
		std::list<Triangle> faces;

		Polytope(
			const ConvexCollider& collider1, const ConvexCollider& collider2,
			std::vector<SupportPoint>& simplex
		);
	};

}}

#endif		// POLYTOPE_H
