#ifndef TRIANGLE_COLLIDER_H
#define TRIANGLE_COLLIDER_H

#include <array>
#include "ConvexCollider.h"

namespace se::physics {

	/**
	 * Class TriangleCollider, it's a ConvexCollider whose vertices form a
	 * Triangle
	 */
	class TriangleCollider : public ConvexCollider
	{
	protected:	// Attributes
		/** The 3 vertices of the TriangleCollider in local coordinates */
		std::array<glm::vec3, 3> mLocalVertices;

		/** The 3 vertices of the TriangleCollider in world coordinates */
		std::array<glm::vec3, 3> mWorldVertices;

		/** The transformation matrix of the TriangleCollider */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new TriangleCollider located at the origin of coordinates
		 *
		 * @param	vertices the vertices of the triangle in local
		 *			coordinates */
		TriangleCollider(const std::array<glm::vec3, 3>& vertices = {}) :
			mLocalVertices(vertices), mWorldVertices(vertices),
			mTransformsMatrix(1.0f) {};

		/** @return	the vertices of the TriangleCollider in local coordinates */
		const std::array<glm::vec3, 3>& getLocalVertices() const
		{ return mLocalVertices; };

		/** Sets the vertices of the TriangleCollider
		 *
		 * @param	vertices the new vertices of the TriangleCollider in local
		 *			coordinates */
		void setLocalVertices(const std::array<glm::vec3, 3>& vertices);

		/** @copydoc Collider::clone() */
		virtual std::unique_ptr<Collider> clone() const override
		{ return std::make_unique<TriangleCollider>(*this); };

		/** @copydoc Collider::setTransforms() */
		void setTransforms(const glm::mat4& transforms) override;

		/** @copydoc Collider::getTransforms() */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @copydoc Collider::getAABB() */
		AABB getAABB() const override;

		/** @copydoc ConvexCollider::getFurthestPointInDirection() */
		void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const override;
	};

}

#endif		// TRIANGLE_COLLIDER_H
