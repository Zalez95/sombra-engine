#ifndef CONVEX_POLYHEDRON_H
#define CONVEX_POLYHEDRON_H

#include "HalfEdgeMesh.h"
#include "ConvexCollider.h"

namespace se::collision {

	/**
	 * Class ConvexPolyhedron, it's a ConvexCollider whose vertices form a
	 * convex shape
	 */
	class ConvexPolyhedron : public ConvexCollider
	{
	protected:	// Attributes
		/** The Mesh in world coordinates */
		HalfEdgeMesh mMesh;

		/** The HEVertices of the ConvexPolyhedron's in local space */
		utils::PackedVector<HEVertex> mLocalVertices;

		/** The transformation matrix of the ConvexPolyhedron */
		glm::mat4 mTransformsMatrix;

		/** If the ConvexPolyhedron has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new ConvexPolyhedron located at the origin of coordinates
		 *
		 * @param	meshData the convex 3D HalfEdgeMesh used to create the
		 *			ConvexPolyhedron. Empty mesh by default
		 * @note	the Mesh must be convex */
		ConvexPolyhedron(const HalfEdgeMesh& meshData = HalfEdgeMesh());

		/** Class destructor */
		virtual ~ConvexPolyhedron() = default;

		/** @return	the convex 3D HalfEdgeMesh of the ConvexPolyhedron in local
		 *			space */
		HalfEdgeMesh getLocalMesh() const;

		/** Sets the convex 3D HalfEdgeMesh of the ConvexPolyhedron
		 *
		 * @param	meshData the convex 3D HalfEdgeMesh in local space used by
		 *			the ConvexPolyhedron */
		void setLocalMesh(const HalfEdgeMesh& meshData);

		/** Updates the scale, translation and orientation of the
		 * ConvexPolyhedron with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the
		 *			ConvexPolyhedron */
		void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			ConvexPolyhedron */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @return	the Axis Aligned Bounding Box that contains the
		 *			ConvexPolyhedron */
		AABB getAABB() const override;

		/** @return	true if the ConvexPolyhedron has been updated since the last
		 *			call to the resetUpdatedState function, false otherwise */
		bool updated() const override { return mUpdated; };

		/** Resets the updated state of the ConvexPolyhedron */
		void resetUpdatedState() override { mUpdated = false; };

		/** Calculates the coordinates of the ConvexPolyhedron's furthest point
		 * in the given direction
		 *
		 * @param	direction the direction towards we want to get the furthest
		 *			point
		 * @param	pointWorld the vector where we are going to store the
		 *			coordinates in world space of ConvexPolyhedron's
		 *			furthest point
		 * @param	pointLocal the vector where we are going to store the
		 *			coordinates in local space of ConvexPolyhedron's
		 *			furthest point */
		void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const override;
	};

}

#endif		// CONVEX_POLYHEDRON_H
