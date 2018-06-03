#ifndef TERRAIN_COLLIDER_H
#define TERRAIN_COLLIDER_H

#include "ConcaveCollider.h"

namespace fe { namespace collision {

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
		const std::vector<float> mHeights;

		/** The number of vertices of the Terrain in the X axis */
		int mXSize;

		/** The number of vertices of the Terrain in the Z axis */
		int mZSize;

		/** The transformation matrix of the TerrainCollider */
		glm::mat4 mTransformsMatrix;

		/** The inverse of the transformation matrix of the TerrainCollider */
		glm::mat4 mInverseTransformsMatrix;

		/** The AABB of the TerrainCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new TerrainCollider located at the origin of
		 * coordinates
		 *
		 * @param	heights the heights (Y axis) of the TerrainCollider's
		 *			vertices in the range [-0.5, 0.5]
		 * @param	xSize the number of vertices in the X axis
		 * @param	zSize the number of vertices in the Z axis */
		TerrainCollider(
			const std::vector<float>& heights, int xSize, int zSize
		);

		/** Class destructor */
		virtual ~TerrainCollider() {};

		/** Updates the scale, translation and orientation of the
		 * TerrainCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the TerrainCollider */
		virtual void setTransforms(const glm::mat4& transforms);

		/** @return	the transformations matrix currently applied to the
		 *			TerrainCollider */
		inline virtual glm::mat4 getTransforms() const
		{ return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			TerrainCollider */
		inline virtual AABB getAABB() const { return mAABB; };

		/** @return a set with the posible overlaping parts of the Collider
		 * with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	a set with Convex parts of the collider that can be
		 *			overlaping with the given AABB */
		virtual std::vector<const ConvexCollider*> getOverlapingParts(
			const AABB& aabb
		) const;
	private:
		/** Calculates the AABB of the TerrainCollider with its local heights
		 * and transforms matrix */
		void calculateAABB();

		/** Creates a local AABB from the given aabb in world space
		 *
		 * @param	aabb the AABB in world space
		 * @return	the new AABB in local space
		 * @note	the AABB in local space could have a larger volume and
		 *			surface than the original one */
		AABB calculateLocalAABB(const AABB& aabb) const;

		/** Checks if the given AABB is inside the given mesh in the Y axis
		 *
		 * @param	aabb the AABB in local space
		 * @param	vertices the vertices of the mesh
		 * @return	true if the AABB (or part of it) is inside the vertices in
		 *			the Y axis, false otherwise */
		bool checkYAxis(
			const AABB& aabb, const std::vector<glm::vec3>& vertices
		) const;
	};

}}

#endif		// TERRAIN_COLLIDER_H
