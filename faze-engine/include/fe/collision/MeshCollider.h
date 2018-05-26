#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "ConcaveCollider.h"
#include "ConvexPolyhedron.h"

namespace fe { namespace collision {

	/**
	 * The algorithm used to split the ConcaveCollider in multiple
	 * ConvexColliders
	 */
	enum class ConvexStrategy
	{
		/** QuickHull algorithm used to calculate the Convex Hull */
		QuickHull,
		/** HACD algorithm used to split a ConcaveCollider in multiple
		 * ConvexColliders */
		HACD
	};


	/**
	 * Class MeshCollider, it's a Collider with a concave shape which will be
	 * stored as a triangle Mesh.
	 */
	class MeshCollider : public ConcaveCollider
	{
	private:	// Attributes
		/** The multiple pieces in which the ConcaveCollider is splited */
		std::vector<ConvexPolyhedron> mConvexParts;

		/** The transformation matrix of the MeshCollider */
		glm::mat4 mTransformsMatrix;

		/** The AABB of the MeshCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new MeshCollider located at the origin of
		 * coordinates
		 *
		 * @param	vertices the vertices of the MeshCollider in local
		 *			space
		 * @param	indices the indices of the triangle mesh of the
		 *			MeshCollider */
		MeshCollider(
			const std::vector<glm::vec3>& vertices,
			const std::vector<unsigned short>& indices,
			ConvexStrategy strategy
		);

		/** Class destructor */
		virtual ~MeshCollider() {};

		/** Updates the scale, translation and orientation of the MeshCollider
		 * with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the MeshCollider */
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
		 * @return	a set with the pointers to the Convex parts of the collider
		 *			that could be overlaping with the given AABB */
		virtual std::vector<const ConvexCollider*> getOverlapingParts(
			const AABB& aabb
		) const;
	private:
		/** Calculates the AABB of the MeshCollider */
		void calculateAABB();

		/** TODO: */
		std::vector<glm::vec3> doQuickHull(
			const std::vector<glm::vec3>& points
		) const;

		/** Calculates the initial simplex needed for calculating the QuickHull
		 * algorithm from the given points
		 *
		 * @param	points the points with which we will calculate the initial simplex
		 * @note	the number of points must be bigger than 3
		 * @return	the initial simplex of the QuickHull algorithm */
		std::vector<glm::vec3> createInitialHull(
			const std::vector<glm::vec3>& points
		) const;

	};

}}

#endif		// MESH_COLLIDER_H
