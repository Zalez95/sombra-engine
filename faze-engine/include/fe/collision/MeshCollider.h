#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "ConcaveCollider.h"

namespace fe { namespace collision {

	/**
	 * Class MeshCollider, a Collider with a concave shape which will be
	 * stored as a triangle Mesh.
	 */
	class MeshCollider : public ConcaveCollider
	{
	private:	// Attributes
		/** The coordinates in local space of the MeshCollider's
		 * mesh vertices */
		const std::vector<glm::vec3> mVertices;

		/** The coordinates in world space of the MeshCollider's
		 * mesh vertices */
		std::vector<glm::vec3> mVerticesWorld;

		/** The indices of the triangle faces of the MeshCollider */
		const std::vector<unsigned short> mIndices;

		/** The transformation matrix of the MeshCollider */
		glm::mat4 mTransformsMatrix;

		/** The AABBs of each triangle in the MeshCollider */
		std::vector<AABB> mTriangleAABBs;

		/** The AABB of the MeshCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new MeshCollider located at the origin of
		 * coordinates
		 *
		 * @param	vertices the vertices of the MeshCollider in local
		 *			space
		 * @param	indices the indices of the faces of the MeshCollider */
		MeshCollider(
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices
		);

		/** Class destructor */
		virtual ~MeshCollider() {};

		/** Updates the translation and orientation of the MeshCollider
		 * with the data of the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			position and orientation of the MeshCollider */
		virtual void setTransforms(const glm::mat4& transforms);

		/** @return	the transformations matrix currently applied to the
		 *			MeshCollider */
		inline virtual glm::mat4 getTransforms() const
		{ return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			MeshCollider */
		inline virtual AABB getAABB() const { return mAABB; };

		/** @return a set with the posible overlaping parts of the Collider
		 * with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	a set with Convex parts of the collider that can be
		 *			overlaping with the given AABB */
		virtual std::vector<ConvexPart> getOverlapingParts(
			const AABB& aabb
		) const;
	private:
		void calculateAABBs();
	};

}}

#endif		// MESH_COLLIDER_H
