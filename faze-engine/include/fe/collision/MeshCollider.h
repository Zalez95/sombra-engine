#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "ConcaveCollider.h"
#include "ConvexPolyhedron.h"

namespace fe { namespace collision {

	/**
	 * Class MeshCollider, it's a Collider with a concave shape stored as a
	 * Half-Edge mesh.
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
		/** Creates a new MeshCollider located at the origin of coordinates
		 * from the given convex pieces
		 *
		 * @param	convexParts the multiple pieces in which the ConcaveCollider
		 *			is splitted */
		MeshCollider(const std::vector<ConvexPolyhedron>& convexParts);

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
