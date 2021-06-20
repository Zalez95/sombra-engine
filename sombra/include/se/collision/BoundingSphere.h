#ifndef BOUNDING_SPHERE_H
#define BOUNDING_SPHERE_H

#include "ConvexCollider.h"

namespace se::collision {

	/**
	 * Class BoundingSphere, a BoundingSphere is a ConvexCollider with the
	 * shape of a Sphere
	 */
	class BoundingSphere : public ConvexCollider
	{
	private:	// Attributes
		/** The Radius of the BoundingSphere */
		float mRadius;

		/** The transformation matrix of the BoundingSphere */
		glm::mat4 mTransformsMatrix;

		/** The inverse of transformation matrix of the BoundingSphere */
		glm::mat4 mInverseTransformsMatrix;

		/** If the BoundingSphere has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new BoundingSphere located at the origin of coordinates
		 *
		 * @param	radius the Radius of the new BoundingSphere */
		BoundingSphere(float radius = 0.0f);

		/** @return	the center of the BoundingSphere in world coordinates */
		glm::vec3 getCenter() const { return glm::vec3(mTransformsMatrix[3]); };

		/** @return	the radius of the BoundingSphere */
		float getRadius() const { return mRadius; };

		/** Sets the radius of the BoundingSphere
		 *
		 * @param	radius the new BoundingSphere radius */
		void setRadius(float radius);

		/** @copydoc Collider::clone() */
		virtual std::unique_ptr<Collider> clone() const override
		{ return std::make_unique<BoundingSphere>(*this); };

		/** Updates the scale, translation and orientation of the BoundingSphere
		 * with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the BoundingSphere */
		void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			BoundingSphere */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @return	the Axis Aligned Bounding Box that contains the
		 *			BoundingSphere */
		AABB getAABB() const override;

		/** @return	true if the BoundingSphere has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		bool updated() const override { return mUpdated; };

		/** Resets the updated state of the BoundingSphere */
		void resetUpdatedState() override { mUpdated = false; };

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
		void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const override;
	};

}

#endif		// BOUNDING_SPHERE_H
