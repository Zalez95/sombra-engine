#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

namespace graphics {
	
	/** Struct AABB, it's a Axis Aligned Bounding Box that holds the maximum
	 * and the minimum coordinates in Global Space of the vertices of the mesh
	 * in each axis of */
	struct AABB
	{
		/** The maximum coordinates of the vertices of the Mesh in Global
		 * Space */
		glm::vec3 mMaximum;

		/** The minimum coordinates of the vertices of the Mesh in Global
		 * Space */
		glm::vec3 mMinimum;
	};

}

#endif		// AABB_H
