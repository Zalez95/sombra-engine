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

#endif		// CAPSULE_H
