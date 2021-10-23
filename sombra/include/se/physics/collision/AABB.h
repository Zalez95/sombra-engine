#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

namespace se::physics {

	/**
	 * Struct AABB, it's an Axis Aligned Bounding Box that holds the maximum
	 * and the minimum coordinates in each axis of a Box in World Space
	 */
	struct AABB
	{
		/** The minimum coordinates in each axis of the Mesh's vertices in
		 * World Space */
		glm::vec3 minimum = glm::vec3(0.0f);

		/** The maximum coordinates in each Axis of the Mesh's vertices in
		 * World Space */
		glm::vec3 maximum = glm::vec3(0.0f);
	};


	/** Creates a new AABB that contains both of the given AABBs
	 *
	 * @param	aabb1 the first of the AABBs
	 * @param	aabb2 the second of the AABBs
	 * @return	the new AABB */
	AABB expand(const AABB& aabb1, const AABB& aabb2);


	/** Checks if the given AABBs are overlapping or not
	 *
	 * @param	aabb1 the first of the fiven AABBs to check
	 * @param	aabb2 the second of the fiven AABBs to check
	 * @param	epsilon the precision of the calculation
	 * @return true if the given AABB overlaps each other, false otherwise */
	bool overlaps(const AABB& aabb1, const AABB& aabb2, float epsilon);


	/** Checks if the given ray intersects the given AABB
	 *
	 * @param	aabb the AABB to test
	 * @param	origin the origin point of the ray
	 * @param	direction the direction vector of the ray
	 * @param	epsilon the precision of the calculation
	 * @return	true if the ray intersects the AABB, false otherwise */
	bool intersects(
		const AABB& aabb,
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
		float epsilon
	);


	/** Creates a new AABB by computing the AABB of the transformed initial AABB
	 *
	 * @param	aabb the initial AABB
	 * @param	transforms the transformation matrix
	 * @return	the new AABB */
	AABB transform(const AABB& aabb, const glm::mat4& transforms);


	/** Calculates the Area of the given AABB
	 *
	 * @param	aabb the AABB to calculate its area
	 * @return	the area of the AABB */
	float calculateArea(const AABB& aabb);

}

#endif		// AABB_H
