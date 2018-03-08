#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

namespace fe { namespace collision {

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

		/** Creates a new AABB */
		AABB() : minimum(0.0f), maximum(0.0f) {};

		/** Creates a new AABB
		 *
		 * @param	minimum the minimum coordinates in each axis of the
		 *			AABB in World Space
		 * @param	maximum the maximum coordinates in each axis of the
		 *			AABB in World Space */
		AABB(const glm::vec3& minimum, const glm::vec3& maximum) :
			minimum(minimum), maximum(maximum) {};

		/** Destructor */
		~AABB() {};

		/** @return true if the current AABB overlaps the given one, false
		 * otherwise */
		bool overlaps(const AABB& other) const;
	};

}}

#endif		// AABB_H
