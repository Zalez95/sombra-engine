#ifndef CAPSULE_H
#define CAPSULE_H

#include "ConvexCollider.h"

namespace se::collision {

	/**
	 * Class Capsule, a Capsule is a ConvexCollider with the shape of a Capsule
	 * or a Cylinder with hemispherical ends.
	 */
	class Capsule : public ConvexCollider
	{
	private:	// Attributes
		/** The Radius of the Capsule */
		float mRadius;

		/** The Height of the Capsule */
		float mHeight;

		/** The transformation matrix of the Capsule */
		glm::mat4 mTransformsMatrix;

		/** The inverse of transformation matrix of the Capsule */
		glm::mat4 mInverseTransformsMatrix;

		/** If the Capsule has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new Capsule located at the origin of coordinates
		 *
		 * @param	radius the Radius of the new Capsule
		 * @param	height the Height of the new Capsule */
		Capsule(float radius = 0.0f, float height = 0.0f);

		/** @return	the radius of the Capsule */
		float getRadius() const { return mRadius; };

		/** Sets the radius of the Capsule
		 *
		 * @param	radius the new radius of the Capsule */
		void setRadius(float radius);

		/** @return	the height of the Capsule */
		float getHeight() const { return mHeight; };

		/** Sets the height of the Capsule
		 *
		 * @param	height the new height of the Capsule */
		void setHeight(float height);

		/** @copydoc Collider::clone() */
		virtual std::unique_ptr<Collider> clone() const override
		{ return std::make_unique<Capsule>(*this); };

		/** Updates the scale, translation and orientation of the Capsule with
		 * the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the Capsule */
		void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			Capsule */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @return	the Axis Aligned Bounding Box that contains the Capsule */
		AABB getAABB() const override;

		/** @return	true if the Capsule has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		bool updated() const override { return mUpdated; };

		/** Resets the updated state of the Capsule */
		void resetUpdatedState() override { mUpdated = false; };

		/** Calculates the coordinates of the Capsule's furthest point in the
		 * given direction
		 *
		 * @param	direction the direction towards we want to get the furthest
		 *			point
		 * @param	pointWorld the vector where we are going to store the
		 *			coordinates in world space of Capsule's furthest point
		 * @param	pointLocal the vector where we are going to store the
		 *			coordinates in local space of Capsule's furthest point */
		void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const override;
	};

}

#endif		// CAPSULE_H
