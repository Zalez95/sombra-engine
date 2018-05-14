#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <array>
#include <glm/glm.hpp>
#include "Edge.h"

namespace fe { namespace collision {

	/**
	 * Struct Triangle
	 */
	struct Triangle
	{
		/** The first Edge of the Triangle */
		Edge ab;

		/** The second Edge of the Triangle */
		Edge bc;

		/** The third Edge of the Triangle */
		Edge ca;

		/** The normal of Triangle */
		glm::vec3 normal;

		/** Creates a Triangle from the given SupportPoints */
		Triangle(SupportPoint* a, SupportPoint* b, SupportPoint* c);

		/** Compares the current Triangle with the given one
		 *
		 * @param	other the Triangle to compare with the current one
		 * @return	true if the Triangles are the same, false otherwise */
		bool operator==(const Triangle& other) const;
	};


	/** Calculates the minimum distance from a point to the given triangle
	 *
	 * @param	p the point whose distance to the triangle we want to know
	 * @param	t1 the first point of the triangle
	 * @param	t2 the second point of the triangle
	 * @param	t3 the third point of the triangle
	 * @return	the minimum distance of p to the triangle between e1, e2
	 *			and e3 */
	float distancePointTriangle(
		const glm::vec3& p,
		const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3
	);


	/** Projects the given point onto the the given 3D triangle 
	 *
	 * @param	point the 3D coordinates of the point in world space
	 * @param	triangle an array with the 3 points of the triangle in
	 *			world space
	 * @param	projectionPrecision the precision of the projected point onto
	 *			the triangle
	 * @param	projectedPoint a reference to the vector where we are going
	 *			to store the coordinates of the projected point in
	 *			barycentric coordinates
	 * @return	true if the point could be projected onto the triangle,
	 *			false otherwise*/
	bool projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		float projectionPrecision, glm::vec3& projectedPoint
	);

}}

#endif		// TRIANGLE_H
