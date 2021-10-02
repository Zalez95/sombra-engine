#ifndef COLLIDER_H
#define COLLIDER_H

#include <memory>
#include <glm/glm.hpp>
#include "AABB.h"

namespace se::physics {

	class RigidBody;


	/**
	 * Class Collider, a Collider is used to store the basic data of an object
	 * that can collide with other Colliders
	 */
	class Collider
	{
	protected:	// Attributes
		/** The parent RigidBody of the Collider */
		RigidBody* mParent = nullptr;

	public:		// Functions
		/** Class destructor */
		virtual ~Collider() = default;

		/** @return	a pointer to a copy of the current Collider */
		virtual std::unique_ptr<Collider> clone() const = 0;

		/** Sets the parent RigidBody of the Collider
		 *
		 * @param	parent the new parent RigidBody of the Collider */
		void setParent(RigidBody* parent) { mParent = parent; };

		/** @return	the parent RigidBody of the Collider, nullptr if it doesn't
		 *			have one */
		RigidBody* getParent() const { return mParent; };

		/** Updates the scale, translation and orientation of the Collider
		 * with the data of the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, position and orientation of the Collider */
		virtual void setTransforms(const glm::mat4& transforms) = 0;

		/** @return	the transformations matrix currently applied to the
		 *			Collider */
		virtual glm::mat4 getTransforms() const = 0;

		/** @return the Axis Aligned Bounding Box that contains the
		 *			Collider */
		virtual AABB getAABB() const = 0;

		/** @return	true if the Collider has been updated since the last call
		 *			to the resetUpdatedState function, false otherwise */
		virtual bool updated() const = 0;

		/** Resets the updated state of the Collider */
		virtual void resetUpdatedState() = 0;
	};

}

#endif		// COLLIDER_H
