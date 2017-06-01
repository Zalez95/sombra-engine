#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <vector>
#include "Contact.h"

namespace physics {

	class Collider;
	class Plane;
	class BoundingSphere;
	class BoundingBox;

	/**
	 * Class CollisionDetector, is the class that calculates the CollisionData
	 * generated from the collision of the Colliders
	 */
	class CollisionDetector
	{
	public:		// Functions
		/** Creates a new CollisionDetector */
		CollisionDetector() {};

		/** Class destructor */
		~CollisionDetector() {};

		/** Returns the data of the collision happened between the given
		 * Colliders
		 * 
		 * @param	collider1 a pointer to the first Collider with which we 
		 *			will calculate the collision data
		 * @param	collider2 a pointer to the second Collider with which we
		 *			will calculate the collision data
		 * @return	the data of the Collision */
		std::vector<Contact> collide(
			const Collider* collider1,
			const Collider* collider2
		) const;
	private:
		/** Returns the data of the collision happened between the given
		 * BoundingSpheres
		 *  
		 * @param	sphere1 a pointer to the first BoundingSphere with which
		 *			we will calculate the data of the collision
		 * @param	sphere2 a pointer to the second BoundingSphere with which
		 *			we will calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSpheres(
			const BoundingSphere* sphere1,
			const BoundingSphere* sphere2
		) const;

		/** Returns the data of the collision happened between the given
		 * BoundingSphere and the given Plane
		 * 
		 * @note	the BoundingSphere can collide with the plane only if it
		 *			crosses the plane in the opposite direction of the
		 *			plane's normal
		 * @param	sphere a pointer to the BoundingSphere with which we will
		 *			calculate the data of the collision
		 * @param	plane a pointer to the plane with which we will calculate
		 *			the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSphereAndPlane(
			const BoundingSphere* sphere,
			const Plane* plane
		) const;

		/** Returns the data of the collision happened between the given
		 * BoundingBoxes
		 *
		 * @param	box1 a pointer to the first BoundingBox with which we
		 * 			will calculate the data of the collision
		 * @param	box2 a pointer to the second BoundingBox with which we
		 * 			will calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideBoxes(
			const BoundingBox* box1,
			const BoundingBox* box2
		) const;
		
		/** Returns the data of the collision happened between the given
		 * BoudingBox and the given Plane
		 * 
		 * @note	the BoundingBox can collide with the plane only if it
		 * 			crosses the plane in the opposite direction of the plane's
		 * 			normal
		 * @param	box a pointer to the BoundingBox with which we will
		 * 			calculate the data of the collision
		 * @param	plane a pointer to the plane with which we will calculate
		 *			the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideBoxAndPlane(
			const BoundingBox* box,
			const Plane* plane
		) const;

		/** Returns the data of the collision happened between the given
		 * BoundingBox and the given BoundingSphere
		 *  
		 * @param	sphere a pointer to the BoundingSphere with which we will
		 *			calculate the data of the collision
		 * @param	box a pointer to the BoundingBox with which we will
		 * 			calculate the data of the collision
		 * @return	the data of the collision */
		std::vector<Contact> collideSphereAndBox(
			const BoundingSphere* sphere,
			const BoundingBox* box
		) const;


	};

}

#endif		// COLLISION_DETECTOR_H
