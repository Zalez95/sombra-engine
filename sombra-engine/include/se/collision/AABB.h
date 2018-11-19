#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

namespace se::collision {

	/**
	 * Struct AABB, it's an Axis Aligned Bounding Box that holds the maximum
	 * and the minimum coordinates in each axis of a Box in World Space
	 */
	struct AABB
	{
		/** The minimum coordinates in each axis of the Mesh's vertices in
		 * World Space */
		glm::vec3 minimum;

		/** The maximum coordinates in each Axis of the Mesh's vertices in
		 * World Space */
		glm::vec3 maximum;
	};


	/** Checks if the given AABBs are overlapping or not
	 *
	 * @param	aabb1 the first of the fiven AABBs to check
	 * @param	aabb2 the second of the fiven AABBs to check
	 * @return true if the given AABB overlaps each other, false otherwise */
	bool overlaps(const AABB& aabb1, const AABB& aabb2);

}

#endif		// AABB_H
