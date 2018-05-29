#ifndef EDGE_H
#define EDGE_H

#include <glm/glm.hpp>

namespace fe { namespace collision {

	class SupportPoint;


	/**
	 * Struct Edge
	 */
	struct Edge
	{
		/** The index of the first point of the Edge */
		int p1;

		/** The index of the second point of the Edge */
		int p2;

		/** Compares the current Edge with the given one
		 *
		 * @param	other the Edge to compare with the current one
		 * @return	true if the Edge joins the same points, false otherwise */
		bool operator==(const Edge& other) const;
	};


	/** Calculates the minimum distance from a point to the given edge
	 *
	 * @param	p the point whose distance to the edge we want to know
	 * @param	e1 the first point of the edge
	 * @param	e2 the second point of the edge
	 * @return	the minimum distance of p to the edge between e1 and e2 */
	float distancePointEdge(
		const glm::vec3& p,
		const glm::vec3& e1, const glm::vec3& e2
	);

}}

#endif		// EDGE_H
