#ifndef COLLIDER_H
#define COLLIDER_H

#include <glm/glm.hpp>

namespace collision {

	struct AABB;


	/** The different types that the collider cons have */
	enum ColliderType
	{
		CONVEX_COLLIDER,
		CONCAVE_COLLIDER
	};


	/**
	 * Class Collider, a Collider is used to store the basic data of an object
	 * that can collide with other Colliders
	 */
	class Collider
	{
	public:		// Functions
		/** Class destructor */
		virtual ~Collider() {};

		/** @return the type of the Collider */
		virtual ColliderType getType() const = 0;

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
	};

}

#endif		// COLLIDER_H
