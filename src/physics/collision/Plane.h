#ifndef PLANE_H
#define PLANE_H

#include "Collider.h"

namespace physics {

	/**
	 * Class Plane, a Plane is a Collider with the shape of a Plane
	 */
	class Plane : public Collider
	{
	private:	// Attributes
		/** The normal vector of the Plane */
		glm::vec3 mNormal;

		/** The distance of the Plane from the origin in the direction of
		 * the normal */
		float mDistance;

	public:		// Functions
		/** Creates a new Plane located at origin of coordinates and pointing
		 * towards the z-axis */
		Plane() : mNormal(0,0,1) {};

		/** Creates a new Plane
		 * 
		 * @param	normal the normal vector of the Plane
		 * @param	distance the distance of the plane from the origin
		 * @note	the normal must be normalized */
		Plane(const glm::vec3& normal, float distance) :
			mNormal(normal), mDistance(distance) {};

		/** Class destructor */
		~Plane() {};

		/** @return	the nomral vector of the Plane  */
		inline glm::vec3 getNormal() const { return mNormal; };

		/** @return	the distance of the Plane from the origin in the direction
		 * of its normal */
		inline float getDistance() const { return mDistance; };

		/** Updates the position and orientation of the Plane with the data of
		 * the given transformation matrix
		 * 
		 * @param	transforms the transformations matrix used to set the
		 *			position of the BoundingSphere */
		virtual void setTransforms(const glm::mat4& transforms);

		/** @return the Axis Aligned Bounding Box that contains the
		 *			Plane */
		virtual AABB getAABB() const;
	};

}

#endif		// PLANE_H
