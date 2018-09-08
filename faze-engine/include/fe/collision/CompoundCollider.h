#ifndef MESH_COLLIDER_H
#define MESH_COLLIDER_H

#include "ConcaveCollider.h"
#include "ConvexPolyhedron.h"

namespace fe { namespace collision {

	/**
	 * Class CompoundCollider, it's a Collider with a concave shape stored as a
	 * Half-Edge mesh.
	 */
	class CompoundCollider : public ConcaveCollider
	{
	private:	// Attributes
		/** The multiple convex pieces in which the ConcaveCollider is
		 * splited */
		std::vector<ConvexPolyhedron> mConvexParts;

		/** The transformation matrix of the CompoundCollider */
		glm::mat4 mTransformsMatrix;

		/** The AABB of the CompoundCollider */
		AABB mAABB;

	public:		// Functions
		/** Creates a new CompoundCollider located at the origin of coordinates
		 * from the given convex pieces
		 *
		 * @param	convexParts the multiple convex pieces in which the
		 *			ConcaveCollider is splitted */
		CompoundCollider(const std::vector<ConvexPolyhedron>& convexParts);

		/** Class destructor */
		~CompoundCollider() {};

		/** Updates the scale, translation and orientation of the
		 * CompoundCollider with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the
		 *			CompoundCollider */
		void setTransforms(const glm::mat4& transforms) override;

		/** @return	the transformations matrix currently applied to the
		 *			CompoundCollider */
		glm::mat4 getTransforms() const override { return mTransformsMatrix; };

		/** @return the Axis Aligned Bounding Box that contains the
		 *			CompoundCollider */
		AABB getAABB() const override { return mAABB; };

		/** @return a set with the posible overlaping parts of the Collider
		 * with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @return	a set with the pointers to the Convex parts of the collider
		 *			that could be overlaping with the given AABB */
		std::vector<const ConvexCollider*> getOverlapingParts(
			const AABB& aabb
		) const override;
	private:
		/** Calculates the AABB of the CompoundCollider with its Convex parts */
		void calculateAABB();
	};

}}

#endif		// MESH_COLLIDER_H
