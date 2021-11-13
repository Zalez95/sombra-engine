#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::utils {

	/** It combines multiple hash values
	 *
	 * @param	seed the previous hash value, it will be updated with the new
	 *			combined hash. Initially it should be 0
	 * @param	v the value to calculate its hash value
	 * @see		https://www.boost.org/doc/libs/1_35_0/doc/html/boost/
	 *			hash_combine_id241013.html */
	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v)
	{ seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2); }


	/** Struct PairHash, used for calculating the hash value of pairs */
	struct PairHash
	{
		/** Calculates the hash value of the given pair
		 *
		 * @param	pair the pair to calculate its hash value
		 * @return	the hash value */
		template <typename T1, typename T2>
		std::size_t operator()(const std::pair<T1, T2>& pair) const
		{
			std::size_t ret = 0;
			hash_combine(ret, pair.first);
			hash_combine(ret, pair.second);
			return ret;
		}
	};


	/** Calculates the pow with the given numbers
	 *
	 * @param	base the base of the pow function
	 * @param	exponent the exponent of the pow function
	 * @return	the value of the base raised to the exponent power */
	template <typename T>
	constexpr T ipow(T base, unsigned int exponent)
	{ return (exponent == 0)? 1 : base * ipow(base, exponent - 1); }


	/** Linearly interpolates the given values
	 *
	 * @param	a the initial value to interpolate
	 * @param	b the final value to interpolate
	 * @param	f the value used for interpolating between a and b */
	template <typename T>
	T lerp(T a, T b, T f)
	{ return a + f * (b - a); }


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


	/** Calculates the signed distance of the given point to the plane
	 *
	 * @param	plane a vec4 that represents the normal vector of the plane
	 *			(xyz) and a distance constant (w)
	 * @param	point the point to calculate its distance
	 * @return	the signed distance of the point */
	float signedDistancePlanePoint(
		const glm::vec4& plane, const glm::vec3& point
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


	/** Calculates the Bounds in world space
	 *
	 * @param	localMin the minimum value in each axis in local space
	 * @param	localMax the maximum value in each axis in local space
	 * @param	worldMatrix the matrix used for transforming from local space
	 *			to world space
	 * @return	a pair with the minimum and maximum in world space */
	std::pair<glm::vec3, glm::vec3> getBoundsWorld(
		const glm::vec3& localMin, const glm::vec3& localMax,
		const glm::mat4& worldMatrix
	);


	/** Decomposes the given transforms matrix into a translation vector, a
	 * quaternion orientation and a scale vector
	 *
	 * @param	transforms the original transformations matrix
	 * @param	translation the vector where the translation will be stored
	 * @param	rotation the quaternion where the rotation will be stored
	 * @param	scale the vector where the scale will be stored */
	void decompose(
		const glm::mat4& transforms,
		glm::vec3& translation, glm::quat& rotation, glm::vec3& scale
	);

}

#endif		// MATH_UTILS_H
