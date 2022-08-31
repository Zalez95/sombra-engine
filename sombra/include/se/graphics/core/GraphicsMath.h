#ifndef GRAPHICS_MATH_H
#define GRAPHICS_MATH_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::graphics {

	/** Calculates the signed distance of the given point to the plane
	 *
	 * @param	plane a vec4 that represents the normal vector of the plane
	 *			(xyz) and a distance constant (w)
	 * @param	point the point to calculate its distance
	 * @return	the signed distance of the point */
	float signedDistancePlanePoint(
		const glm::vec4& plane, const glm::vec3& point
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

#endif		// GRAPHICS_MATH_H
