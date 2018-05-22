#ifndef CONVEX_POLYHEDRON_H
#define CONVEX_POLYHEDRON_H

#include <vector>
#include "ConvexCollider.h"

namespace fe { namespace collision {

	/**
	 * Class ConvexPolyhedron, it's a ConvexCollider whose vertices form a
	 * convex shape
	 */
	class ConvexPolyhedron : public ConvexCollider
	{
	protected:	// Attributes
		/** The coordinates in local space of the ConvexPolyhedron's
		 * vertices */
		const std::vector<glm::vec3> mVertices;

		/** The coordinates in world space of the ConvexPolyhedron's
		 * vertices */
		std::vector<glm::vec3> mVerticesWorld;

		/** The transformation matrix of the ConvexPolyhedron */
		glm::mat4 mTransformsMatrix;

	public:		// Functions
		/** Creates a new ConvexPolyhedron located at the origin of coordinates
		 *
		 * @param	vertices the vertices of the ConvexPolyhedron in local
		 *			space */
		ConvexPolyhedron(const std::vector<glm::vec3>& vertices);

		/** Class destructor */
		~ConvexPolyhedron() {};

		/** Updates the scale, translation and orientation of the
		 * ConvexPolyhedron with the given transformations matrix
		 *
		 * @param	transforms the transformations matrix used to update the
		 *			scale, translation and orientation of the
		 *			ConvexPolyhedron */
		virtual void setTransforms(const glm::mat4& transforms);

		/** @return	the transformations matrix currently applied to the
		 *			ConvexPolyhedron */
		inline virtual glm::mat4 getTransforms() const
		{ return mTransformsMatrix; };

		/** @return	the Axis Aligned Bounding Box that contains the
		 *			ConvexPolyhedron */
		virtual AABB getAABB() const;

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
		virtual void getFurthestPointInDirection(
			const glm::vec3& direction,
			glm::vec3& pointWorld, glm::vec3& pointLocal
		) const;
	};

}}

#endif		// CONVEX_POLYHEDRON_H
