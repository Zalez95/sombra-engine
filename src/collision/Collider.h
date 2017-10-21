#ifndef COLLIDER_H
#define COLLIDER_H

#include <glm/glm.hpp>

namespace collision {

	struct AABB;


	/**
	 * Class Collider, a Collider is used to store the basic data of an object
	 * that can collide with other Colliders
	 */
	class Collider
	{
	public:		// Functions
		/** Class destructor */
		virtual ~Collider() {};

		/** Updates the translation and orientation of the Collider with the
		 * data of the given transformations matrix
		 * 
		 * @param	transforms the transformations matrix used to update the
		 *			position and orientation of the Collider */
		virtual void setTransforms(const glm::mat4& transforms) = 0;

		/** @return	the transformations matrix currently applied to the
		 *			Collider */
		virtual glm::mat4 getTransforms() const = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			Collider */
		virtual AABB getAABB() const = 0;

		/** Calculates the coordinates of the Collider's furthest point in the
		 * given direction
		 * 
		 * @param	direction the direction towards we want to get the furthest
		 *			point
		 * @param	pointWorld the vector where we are going to store the
		 *			coordinates in world space of Collider's furthest point
		 * @param	pointLocal the vector where we are going to store the
		 *			coordinates in local space of Collider's furthest point */
		virtual void getFurthestPointInDirection(
		   	const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const = 0;
	};

}

#endif		// COLLIDER_H
