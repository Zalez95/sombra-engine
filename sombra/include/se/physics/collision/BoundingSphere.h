#ifndef BOUNDING_SPHERE_H
#define BOUNDING_SPHERE_H

#include "ConvexCollider.h"

namespace se::physics {

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

		/** @copydoc Collider::setTransforms() */
		void setTransforms(const glm::mat4& transforms) override;

		/** @copydoc Collider::getTransforms() */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @copydoc Collider::getAABB() */
		AABB getAABB() const override;

		/** @copydoc Collider::updated() */
		bool updated() const override { return mUpdated; };

		/** @copydoc Collider::resetUpdatedState() */
		void resetUpdatedState() override { mUpdated = false; };

		/** @copydoc ConvexCollider::getFurthestPointInDirection() */
		void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const override;
	};

}

#endif		// BOUNDING_SPHERE_H
