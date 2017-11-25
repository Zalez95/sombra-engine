#ifndef BOUNDING_SPHERE_H
#define BOUNDING_SPHERE_H

#include "ConvexCollider.h"
#include <glm/gtx/quaternion.hpp>

namespace collision {

	/**
	 * Class BoundingSphere, a BoundingSphere is a ConvexCollider with the
	 * shape of a Sphere
	 */
	class BoundingSphere : public ConvexCollider
	{
	private:	// Attributes
		/** The Radius if the BoundingSphere */
		float mRadius;

		/** The transformation matrix of the BoundingSphere */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new BoundingSphere located at the origin of coordinates
		 * 
		 * @param	radius the Radius of the new BoundingSphere */
		BoundingSphere(float radius) : mRadius(radius) {};

		/** Class destructor */
		~BoundingSphere() {};

		/** @return	the center of the BoundingSphere in world coordinates */
		inline glm::vec3 getCenter() const
		{ return glm::vec3(mTransformsMatrix[3]); };

		/** @return	the radius of the BoundingSphere */
		inline float getRadius() const { return mRadius; };
		
		/** Updates the position of the BoundingSphere with the data of the
		 * given transformation matrix
		 * 
		 * @param	transforms the transformations matrix used to set the
		 *			position of the BoundingSphere */
		virtual void setTransforms(const glm::mat4& transforms);
		
		/** @return	the transformations matrix currently applied to the
		 *			BoundingSphere */
		inline virtual glm::mat4 getTransforms() const
		{ return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			BoundingBox */
		virtual AABB getAABB() const;

		/** Calculates the coordinates of the BoundingSphere's furthest point
		 * in the given direction
		 * 
		 * @param	direction the direction towards we want to get the furthest
		 *			point
		 * @param	pointWorld the vector where we are going to store the
		 *			coordinates in world space of BoundingSphere's
		 *			furthest point
		 * @param	pointLocal the vector where we are going to store the
		 *			coordinates in local space of BoundingSphere's
		 *			furthest point */
		virtual void getFurthestPointInDirection(
		   	const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const;
	};

}

#endif		// BOUNDING_SPHERE_H
