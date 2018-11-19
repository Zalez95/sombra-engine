#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <array>
#include <vector>
#include <glm/glm.hpp>

namespace se::collision {

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


	/** Calculates the closest point to p in the given plane
	 *
	 * @param	p the point
	 * @param	planePoints three of the points of the plane
	 * @return	the closest points in the plane to the given point p */
	glm::vec3 getClosestPointInPlane(
		const glm::vec3& p,
		const std::array<glm::vec3, 3>& planePoints
	);


	/** Projects the given point on the given plane along th egiven direction
	 *
	 * @param	point the point to project
	 * @param	direction the direction along we want to project the point
	 * @param	planePoint a point in the plane
	 * @param	planeNormal the normal vector of the plane
	 * @return	a pair with a flag that tells if the point could be projected
	 *			and the 3D coordinates of the projected point */
	std::pair<bool, glm::vec3> projectPointInDirection(
		const glm::vec3& point, const glm::vec3& direction,
		const glm::vec3& planePoint, const glm::vec3& planeNormal
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


	/** Calculates the area of the given triangle
	 *
	 * @param	triangle an array with the 3 points of the triangle in 3D
	 * @return	the area of the triangle */
	float calculateTriangleArea(const std::array<glm::vec3, 3>& triangle);

}

#endif		// TRIANGLE_H
