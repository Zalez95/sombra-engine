#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "ConcaveCollider.h"
#include "ConvexPolyhedron.h"

namespace fe { namespace collision {

	/**
	 * The algorithm used to transform the concave MeshCollider in a/multiple
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
	 * Class MeshCollider, it's a Collider with a concave shape stored as a
	 * Half-Edge mesh.
	 */
	class MeshCollider : public ConcaveCollider
	{
	private:	// Nested types
		using EdgeMatrix = std::vector<std::vector<bool>>;

		/** The epsilon used to calculate the convex parts of the
		 * MeshCollider */
		static constexpr float sEpsilon = 0.001f;

	private:	// Attributes
		/** The minimum concavity needed for HACD algorithm */
		static constexpr float sMinimumConcavity = 0.5f;

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
		 * @param	meshData the mesh of the MeshCollider
		 * @param	strategy the algorithm used for the convex */
		MeshCollider(const HalfEdgeMesh& meshData, ConvexStrategy strategy);

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
		/** Calculates the AABB of the MeshCollider with its Convex parts */
		void calculateAABB();
	};

}}

#endif		// MESH_COLLIDER_H
