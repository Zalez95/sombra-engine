#ifndef BOUNDING_SPHERE_H
#define BOUNDING_SPHERE_H

#include "Collider.h"

namespace physics {

	/**
	 * Class BoundingSphere, a BoundingSphere is a Collider with the
	 * shape of a Sphere
	 */
	class BoundingSphere : public Collider
	{
	private:	// Attributes
		/** The center of the BoundingSphere in world coordinates */
		glm::vec3 mCenter;

		/** The Radius if the BoundingSphere */
		float mRadius;

	public:		// Functions
		/** Creates a new BoundingSphere located at the origin of coordinates
		 * 
		 * @param	radius the Radius of the new BoundingSphere */
		BoundingSphere(float radius) : mRadius(radius) {};

		/** Creates a new BoundingSphere
		 * 
		 * @param	center the Center position of the new BoundingSphere
		 * @param	radius the Radius of the new BoundingSphere */
		BoundingSphere(const glm::vec3& center, float radius) :
			mCenter(center), mRadius(radius) {};

		/** Class destructor */
		~BoundingSphere() {};

		/** @return	the center of the BoundingSphere in world coordinates */
		inline glm::vec3 getCenter() const { return mCenter; };

		/** @return	the radius of the BoundingSphere */
		inline float getRadius() const { return mRadius; };
		
		/** Updates the position of the BoundingSphere with the data of the
		 * given transformation matrix
		 * 
		 * @param	transforms the transformations matrix used to set the
		 *			position of the BoundingSphere */
		virtual void setTransforms(const glm::mat4& transforms);
	};

}

#endif		// BOUNDING_SPHERE_H