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
	};


	/** Calculates the closest point to p in the given plane
	 *
	 * @param	p the point
	 * @param	planePoints three of the points of the plane
	 * @return	the closest points in the plane to the given point p */
	glm::vec3 getClosestPointInPlane(
		const glm::vec3& p,
		const std::array<glm::vec3, 3>& planePoints
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
	 *			false otherwise */
	bool projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		float projectionPrecision, glm::vec3& projectedPoint
	);

}}

#endif		// TRIANGLE_H
