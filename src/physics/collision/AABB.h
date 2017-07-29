#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

namespace physics {
	
	/** 
	 * Struct AABB, it's an Axis Aligned Bounding Box that holds the maximum
	 * and the minimum coordinates in each axis of a Box in World Space
	 */
	struct AABB
	{
		/** The minimum coordinates of the vertices of the Mesh in World
		 * Space */
		glm::vec3 mMinimum;

		/** The maximum coordinates of the vertices of the Mesh in World
		 * Space */
		glm::vec3 mMaximum;

		/** Creates a new AABB
		 * 
		 * @param	minimum the minimum coordinates in each axis of the
		 *			AABB in World Space
		 * @param	maximum the maximum coordinates in each axis of the
		 *			AABB in World Space */
		AABB(const glm::vec3& minimum, const glm::vec3& maximum)
			: mMinimum(minimum), mMaximum(maximum) {};
		
		/** Destructor */
		~AABB() {};
	};

}

#endif		// AABB_H
