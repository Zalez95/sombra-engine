#ifndef COLLIDER_H
#define COLLIDER_H

#include <glm/glm.hpp>

namespace physics {

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
	};

}

#endif		// COLLIDER_H
