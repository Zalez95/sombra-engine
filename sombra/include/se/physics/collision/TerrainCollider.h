#ifndef TERRAIN_COLLIDER_H
#define TERRAIN_COLLIDER_H

#include "ConcaveCollider.h"

namespace se::physics {

	/**
	 * Class TerrainCollider, it's a Collider used to represent a terrain mesh.
	 * The triangles of the terrain mesh will be generated from the
	 * heights of its vertices in Clockwise order, it means that the triangle
	 * grid will always be created like:
	 * z
	 * · — ·
	 * | / |
	 * · — · x
	 */
	class TerrainCollider : public ConcaveCollider
	{
	private:	// Attributes
		/** The positions in the Y axis of the TerrainCollider's vertices in
		 * the range [-0.5, 0.5] */
		std::vector<float> mHeights;

		/** The number of vertices of the Terrain in the X axis */
		std::size_t mXSize;

		/** The number of vertices of the Terrain in the Z axis */
		std::size_t mZSize;

		/** The transformation matrix of the TerrainCollider */
		glm::mat4 mTransformsMatrix;

		/** The inverse of the transformation matrix of the TerrainCollider */
		glm::mat4 mInverseTransformsMatrix;

		/** The AABB of the TerrainCollider */
		AABB mAABB;

		/** If the TerrainCollider has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new TerrainCollider located at the origin of
		 * coordinates */
		TerrainCollider();

		/** @return	the number of vertices in the X axis */
		std::size_t getXSize() const { return mXSize; };

		/** @return	the number of vertices in the Z axis */
		std::size_t getZSize() const { return mZSize; };

		/** @return	the Heights of the TerrainCollider */
		const float* getHeights() const { return mHeights.data(); };

		/** Sets the height data of the TerrainCollider
		 *
		 * @param	heights the heights (Y axis) of the TerrainCollider's
		 *			vertices in the range [-0.5, 0.5]
		 * @param	xSize the number of vertices in the X axis
		 * @param	zSize the number of vertices in the Z axis */
		void setHeights(
			const float* heights,
			std::size_t xSize, std::size_t zSize
		);

		/** @copydoc Collider::clone() */
		virtual std::unique_ptr<Collider> clone() const override
		{ return std::make_unique<TerrainCollider>(*this); };

		/** @copydoc Collider::setTransforms() */
		virtual void setTransforms(const glm::mat4& transforms) override;

		/** @copydoc Collider::getTransforms() */
		virtual glm::mat4 getTransforms() const override
		{ return mTransformsMatrix; };

		/** @copydoc Collider::getAABB() */
		virtual AABB getAABB() const override { return mAABB; };

		/** @copydoc Collider::updated() */
		virtual bool updated() const override { return mUpdated; };

		/** @copydoc Collider::resetUpdatedState() */
		virtual void resetUpdatedState() override { mUpdated = false; };

		/** @copydoc ConcaveCollider::processOverlapingParts() */
		virtual void processOverlapingParts(
			const AABB& aabb, const ConvexShapeCallback& callback
		) const override;
	private:
		/** Calculates the AABB of the TerrainCollider with its local heights
		 * and transforms matrix */
		void calculateAABB();

		/** Checks if the given AABB is inside the given triangle in the Y axis
		 *
		 * @param	aabb the AABB in local space
		 * @param	vertices the vertices of the triangle
		 * @return	true if the AABB (or part of it) is inside the triangle in
		 *			the Y axis, false otherwise */
		bool checkYAxis(
			const AABB& aabb, const std::array<glm::vec3, 3>& vertices
		) const;
	};

}

#endif		// TERRAIN_COLLIDER_H
