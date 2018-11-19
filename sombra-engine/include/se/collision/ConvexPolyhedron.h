#ifndef CONVEX_POLYHEDRON_H
#define CONVEX_POLYHEDRON_H

#include <vector>
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
		ContiguousVector<HEVertex> mLocalVertices;

		/** The transformation matrix of the ConvexPolyhedron */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new ConvexPolyhedron located at the origin of coordinates
		 *
		 * @param	meshData the Half-Edge Mesh used to create the
		 *			ConvexPolyhedron
		 * @note	the Mesh must be convex */
		ConvexPolyhedron(const HalfEdgeMesh& meshData) :
			mMesh(meshData), mLocalVertices(meshData.vertices),
			mTransformsMatrix(1.0f) {};

		/** Class destructor */
		~ConvexPolyhedron() {};

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
