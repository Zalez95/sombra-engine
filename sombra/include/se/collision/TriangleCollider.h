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

		/** If the TriangleCollider has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new TriangleCollider located at the origin of coordinates
		 *
		 * @param	vertices the vertices of the triangle in local
		 *			coordinates */
		TriangleCollider(const std::array<glm::vec3, 3>& vertices = {}) :
			mLocalVertices(vertices), mWorldVertices(vertices),
			mTransformsMatrix(1.0f), mUpdated(true) {};

		/** @return	the vertices of the TriangleCollider in local coordinates */
		const std::array<glm::vec3, 3>& getLocalVertices() const
		{ return mLocalVertices; };

		/** Sets the vertices of the TriangleCollider
		 *
		 * @param	vertices the new vertices of the TriangleCollider in local
		 *			coordinates */
		void setLocalVertices(const std::array<glm::vec3, 3>& vertices);

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

		/** @return	true if the TriangleCollider has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		bool updated() const override { return mUpdated; };

		/** Resets the updated state of the TriangleCollider */
		void resetUpdatedState() override { mUpdated = false; };

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
