#ifndef COLLISION_MATH_H
#define COLLISION_MATH_H

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::physics {

	/** Calculates the closest point to p in the given edge
	 *
	 * @param	p the point
	 * @param	e1 the first point of the edge
	 * @param	e2 the second point of the edge
	 * @return	the closest point in the plane to the given point p */
	glm::vec3 getClosestPointInEdge(
		const glm::vec3& p,
		const glm::vec3& e1, const glm::vec3& e2
	);


	/** Projects the given point onto the the given 3D edge
	 *
	 * @param	point the 3D coordinates of the point
	 * @param	edge an array with the 2 points of the edge
	 * @param	projectionPrecision the precision of the projected point onto
	 *			the edge
	 * @return	a pair with a boolean that tells if the projected point lies
	 *			within the edge and the projected point in barycentric
	 *			coordinates */
	std::pair<bool, glm::vec2> projectPointOnEdge(
		const glm::vec3& point, const std::array<glm::vec3, 2>& edge,
		float projectionPrecision
	);


	/** Calculates the closest point to p in the given plane
	 *
	 * @param	p the point
	 * @param	planePoints three of the points of the plane
	 * @return	the closest point in the plane to the given point p */
	glm::vec3 getClosestPointInPlane(
		const glm::vec3& p,
		const std::array<glm::vec3, 3>& planePoints
	);


	/** Calculates the intersection between the given ray and the given plane
	 *
	 * @param	origin the origin of the ray
	 * @param	direction the direction of the ray
	 * @param	planePoint a point in the plane
	 * @param	planeNormal the normal vector of the plane
	 * @param	intersectionPrecision the precision of the intersection between
	 *			the ray and the plane
	 * @return	a pair with a flag that tells if the point could be projected
	 *			and the 3D coordinates of the projected point */
	std::pair<bool, glm::vec3> rayPlaneIntersection(
		const glm::vec3& origin, const glm::vec3& direction,
		const glm::vec3& planePoint, const glm::vec3& planeNormal,
		float intersectionPrecision
	);


	/** Projects the given point onto the the given 3D triangle
	 *
	 * @param	point the 3D coordinates of the point
	 * @param	triangle an array with the 3 points of the triangle
	 * @param	projectionPrecision the precision of the projected point onto
	 *			the triangle
	 * @return	a pair with a boolean that tells if the projected point lies
	 *			within the triangle and the projected point in barycentric
	 *			coordinates */
	std::pair<bool, glm::vec3> projectPointOnTriangle(
		const glm::vec3& point, const std::array<glm::vec3, 3>& triangle,
		float projectionPrecision
	);


	/** Calculates the area of the given triangle
	 *
	 * @param	triangle an array with the 3 points of the triangle in 3D
	 * @return	the area of the triangle */
	float calculateTriangleArea(const std::array<glm::vec3, 3>& triangle);


	/** Checks if the given triangles are the same or not
	 *
	 * @param	triangle1 the first triangle
	 * @param	triangle2 the second triangle
	 * @param	epsilon the epsilon value used for comparing both triangles
	 * @return	true if both triangles are the same, false otherwise */
	bool compareTriangles(
		const std::array<glm::vec3, 3>& triangle1,
		const std::array<glm::vec3, 3>& triangle2,
		float epsilon
	);

}

#endif		// COLLISION_MATH_H
