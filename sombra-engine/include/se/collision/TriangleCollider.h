#ifndef TRIANGLE_COLLIDER_H
#define TRIANGLE_COLLIDER_H

#include <array>
#include "ConvexCollider.h"

namespace se::collision {

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
		TriangleCollider(const std::array<glm::vec3, 3>& vertices) :
			mLocalVertices(vertices), mWorldVertices(vertices),
			mTransformsMatrix(1.0f) {};

		/** Updates the scale, translation and orientation of the
		 * TriangleCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the
		 *			TriangleCollider */
		void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			TriangleCollider */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @return	the Axis Aligned Bounding Box that contains the
		 *			TriangleCollider */
		AABB getAABB() const override;

		/** Calculates the coordinates of the TriangleCollider's furthest point
		 * in the given direction
		 *
		 * @param	direction the direction towards we want to get the furthest
		 *			point
		 * @param	pointWorld the vector where we are going to store the
		 *			coordinates in world space of TriangleCollider's
		 *			furthest point
		 * @param	pointLocal the vector where we are going to store the
		 *			coordinates in local space of TriangleCollider's
		 *			furthest point */
		void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const override;
	};

}

#endif		// TRIANGLE_COLLIDER_H
